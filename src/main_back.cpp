#include "common.hpp"
#include "dock_panel.hpp"
#include "settings.hpp"
#include "right_panel.hpp"
#include "left_panel.hpp"
#include <QMoveEvent>
#include <QCloseEvent>

// ============================================================================
// 1. БЛОК КАРТОЧКИ С РЕДАКТИРОВАНИЕМ ПРЯМО ПО КЛИКУ НА ПОЛЕ
//    Синхронизирован с немодальным окном «Подробнее» (не перезаписывает extendedDesc)
// ============================================================================
class CardBlockWidget : public QFrame {
private:
    Card cardData;
    DatabaseManager& db;
    double hoverProgress = 0.0;
    QVariantAnimation* hoverAnim = nullptr;
    QGraphicsDropShadowEffect* shadowEffect = nullptr;
    QTimer* autoSaveTimer = nullptr;

    QLineEdit* titleEdit = nullptr;
    QPlainTextEdit* codeEdit = nullptr;
    ImagePasteTextEdit* descEdit = nullptr;
    NeonButton* detailsBtn = nullptr;

    std::string lastSavedTitle;
    std::string lastSavedCode;
    std::string lastSavedDesc;

public:
    long long cardId() const { return cardData.id; }

    void refreshDetailsDotFromDb() {
        for (const auto& c : db.getAllCards()) {
            if (c.id == cardData.id) {
                cardData.extendedDesc = c.extendedDesc;
                if (detailsBtn) {
                    bool hasDetails = !RichTextCodec::toPlainText(cardData.extendedDesc).empty();
                    detailsBtn->setText(hasDetails ? "📝 Подробнее ●" : "📝 Подробнее");
                }
                break;
            }
        }
    }

    void updateShadowColor(double hover) {
        if (!shadowEffect) return;
        int alpha = static_cast<int>(95 + hover * 75);
        if (cardData.pinned) {
            shadowEffect->setColor(QColor(245, 158, 11, alpha));
        } else if (g_cfg.isMaleTheme) {
            shadowEffect->setColor(QColor(0, 195, 255, alpha));
        } else {
            shadowEffect->setColor(QColor(219, 39, 119, alpha));
        }
    }

    CardBlockWidget(const Card& c,
                    DatabaseManager& dbRef,
                    std::function<void(Card)> onOpenDetails,
                    std::function<void(Card)> onTogglePin,
                    std::function<void(Card)> onDeleteCard,
                    QWidget* parent = nullptr)
    : QFrame(parent), cardData(c), db(dbRef)
    {
        int fixedW = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int fixedH = std::clamp(CfgBridge::getCardFixedHeight(), 240, 950);
        double scale = static_cast<double>(fixedW) / 415.0;

        setFixedSize(fixedW, fixedH);
        setAttribute(Qt::WA_Hover, true);

        int innerContentWidth = fixedW - 40;

        QVBoxLayout* layout = new QVBoxLayout(this);
        int pad = std::max(14, static_cast<int>(20 * scale));
        layout->setContentsMargins(pad, pad + 6, pad, pad);
        layout->setSpacing(std::max(6, static_cast<int>(8 * scale)));

        autoSaveTimer = new QTimer(this);
        autoSaveTimer->setSingleShot(true);
        autoSaveTimer->setInterval(500);

        QHBoxLayout* headerRow = new QHBoxLayout();
        headerRow->setContentsMargins(2, 0, 2, 0);
        headerRow->setSpacing(6);

        titleEdit = new QLineEdit(QString::fromStdString(c.title), this);
        QFont tf = font();
        tf.setPointSize(g_cfg.uiFontSize + 1);
        tf.setBold(true);
        titleEdit->setFont(tf);
        titleEdit->setToolTip("Кликните, чтобы сразу изменить заголовок карточки");
        NeonInputFrame* titleFrame = new NeonInputFrame(titleEdit, QColor("#38bdf8"), this);
        titleFrame->setMaximumWidth(innerContentWidth - 68);

        int btnSz = std::max(26, static_cast<int>(30 * scale));
        QPushButton* pinBtn = new QPushButton(c.pinned ? "★" : "☆", this);
        QPushButton* delBtn = new QPushButton("×", this);
        pinBtn->setToolTip("Закрепить карточку");
        delBtn->setToolTip("Удалить карточку");
        pinBtn->setFixedSize(btnSz, btnSz);
        delBtn->setFixedSize(btnSz, btnSz);

        headerRow->addWidget(titleFrame, 1);
        headerRow->addWidget(pinBtn);
        headerRow->addWidget(delBtn);

        QString displaySec = QString::fromStdString(c.section);
        displaySec.replace("/", " ▸ ");
        QLabel* badgeLbl = new QLabel(QString::fromStdString(c.chapter) + "  •  " + displaySec, this);
        QFont bf = font();
        bf.setPointSize(std::max(8, g_cfg.uiFontSize - 1));
        bf.setBold(true);
        badgeLbl->setFont(bf);
        badgeLbl->setWordWrap(true);
        badgeLbl->setContentsMargins(4, 0, 4, 2);
        badgeLbl->setMaximumWidth(innerContentWidth);
        QPalette bPal = badgeLbl->palette();
        bPal.setColor(QPalette::WindowText, getModernColorForName(c.section.empty() ? c.chapter : c.section));
        badgeLbl->setPalette(bPal);

        QHBoxLayout* codeTopRow = new QHBoxLayout();
        codeTopRow->setContentsMargins(4, 0, 4, 0);
        QLabel* codeLangLbl = new QLabel("КОД / КОМАНДА", this);
        codeLangLbl->setFont(bf);
        QPushButton* copyCodeBtn = new QPushButton("Копировать", this);
        copyCodeBtn->setFixedHeight(std::max(22, static_cast<int>(24 * scale)));
        codeTopRow->addWidget(codeLangLbl);
        codeTopRow->addStretch(1);
        codeTopRow->addWidget(copyCodeBtn);

        codeEdit = new QPlainTextEdit(QString::fromStdString(c.code), this);
        codeEdit->setFont(QFont(MONO_FONT_FAMILY, std::max(9, CfgBridge::getEditorFontSize(g_cfg))));
        NeonInputFrame* codeBox = new NeonInputFrame(codeEdit, QColor("#60a5fa"), this);
        codeBox->setFixedHeight(static_cast<int>(82 * scale));

        descEdit = new ImagePasteTextEdit(this);
        descEdit->setToolTip("Кликните для редактирования. Нажмите " + g_cfg.hkFormatPopup + " для вызова док-панели у курсора");
        descEdit->blockSignals(true);
        RichTextCodec::deserialize(c.description, descEdit, static_cast<int>(240 * scale));
        descEdit->blockSignals(false);
        NeonInputFrame* descBox = new NeonInputFrame(descEdit, QColor("#38bdf8"), this);

        lastSavedTitle = titleEdit->text().trimmed().toStdString();
        lastSavedCode  = codeEdit->toPlainText().toStdString();
        lastSavedDesc  = RichTextCodec::serialize(descEdit->document());

        QHBoxLayout* footerRow = new QHBoxLayout();
        footerRow->setContentsMargins(0, 2, 0, 0);

        bool hasDetails = !RichTextCodec::toPlainText(c.extendedDesc).empty();
        detailsBtn = new NeonButton(
            hasDetails ? "📝 Подробнее ●" : "📝 Подробнее",
            g_cfg.isMaleTheme ? QColor("#38bdf8") : QColor("#f472b6"),
                                    this
        );
        detailsBtn->setMaximumWidth(innerContentWidth);
        detailsBtn->setFixedHeight(std::max(28, static_cast<int>(32 * scale)));
        footerRow->addWidget(detailsBtn);

        layout->addLayout(headerRow);
        layout->addWidget(badgeLbl);
        layout->addLayout(codeTopRow);
        layout->addWidget(codeBox);
        layout->addWidget(descBox, 1);
        layout->addLayout(footerRow);

        connect(autoSaveTimer, &QTimer::timeout, [this]() {
            std::string newTitle = titleEdit->text().trimmed().toStdString();
            std::string newCode  = codeEdit->toPlainText().toStdString();
            std::string newDesc  = RichTextCodec::serialize(descEdit->document());

            if (newTitle == lastSavedTitle && newCode == lastSavedCode && newDesc == lastSavedDesc) {
                return;
            }

            // Подтягиваем актуальное подробное описание из БД, чтобы не затереть текст в открытом окне «Подробнее»
            for (const auto& fresh : db.getAllCards()) {
                if (fresh.id == cardData.id) {
                    cardData.extendedDesc = fresh.extendedDesc;
                    break;
                }
            }

            lastSavedTitle       = newTitle;
            lastSavedCode        = newCode;
            lastSavedDesc        = newDesc;
            cardData.title       = newTitle;
            cardData.code        = newCode;
            cardData.description = newDesc;
            db.saveCard(cardData, true);
        });

        connect(titleEdit, &QLineEdit::textEdited, [this]() {
            if (titleEdit->text().trimmed().toStdString() != lastSavedTitle) {
                autoSaveTimer->start();
            }
        });

        connect(codeEdit, &QPlainTextEdit::textChanged, [this]() {
            if (codeEdit->toPlainText().toStdString() != lastSavedCode) {
                autoSaveTimer->start();
            }
        });

        connect(descEdit, &QTextEdit::textChanged, [this]() {
            if (RichTextCodec::serialize(descEdit->document()) != lastSavedDesc) {
                autoSaveTimer->start();
            }
        });

        if (g_cfg.enableShadows) {
            shadowEffect = new QGraphicsDropShadowEffect(this);
            shadowEffect->setBlurRadius(g_cfg.shadowBlurRadius);
            shadowEffect->setOffset(0, 8);
            updateShadowColor(0.0);
            setGraphicsEffect(shadowEffect);
        }

        hoverAnim = new QVariantAnimation(this);
        hoverAnim->setDuration(220);
        connect(hoverAnim, &QVariantAnimation::valueChanged, [this, layout, pad](const QVariant& val) {
            hoverProgress = val.toDouble();
            int liftOffset = static_cast<int>(hoverProgress * 6.0);
            layout->setContentsMargins(pad, pad + 6 - liftOffset, pad, pad + liftOffset);
            if (shadowEffect) {
                shadowEffect->setBlurRadius(g_cfg.shadowBlurRadius + hoverProgress * 12.0);
                shadowEffect->setOffset(0, 8 + hoverProgress * 5.0);
                updateShadowColor(hoverProgress);
            }
            update();
        });

        connect(copyCodeBtn, &QPushButton::clicked, [this, copyCodeBtn]() {
            QApplication::clipboard()->setText(codeEdit->toPlainText());
            copyCodeBtn->setText("Скопировано!");
        });
        connect(detailsBtn, &QPushButton::clicked, [this, onOpenDetails]() {
            for (const auto& fresh : db.getAllCards()) {
                if (fresh.id == cardData.id) {
                    cardData = fresh;
                    break;
                }
            }
            onOpenDetails(cardData);
        });
        connect(pinBtn,     &QPushButton::clicked, [this, onTogglePin]()   { onTogglePin(cardData); });
        connect(delBtn,     &QPushButton::clicked, [this, onDeleteCard]()  { onDeleteCard(cardData); });
    }

protected:
    void enterEvent(QEvent* event) override {
        QFrame::enterEvent(event);
        if (g_cfg.enableHoverFloat && hoverAnim) {
            hoverAnim->stop();
            hoverAnim->setStartValue(hoverProgress);
            hoverAnim->setEndValue(1.0);
            hoverAnim->start();
        }
    }

    void leaveEvent(QEvent* event) override {
        QFrame::leaveEvent(event);
        if (g_cfg.enableHoverFloat && hoverAnim) {
            hoverAnim->stop();
            hoverAnim->setStartValue(hoverProgress);
            hoverAnim->setEndValue(0.0);
            hoverAnim->start();
        }
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        double lift = hoverProgress * 6.0;
        QRectF cardRect = QRectF(this->rect()).adjusted(4, 8 - lift, -4, -4 - lift);

        int alpha = g_cfg.cardOpacity;
        QColor baseCol = cardData.pinned ? g_cfg.bgCardPinned : g_cfg.bgCardNormal;
        QColor c1 = baseCol;
        QColor c2 = baseCol.darker(135);
        c1.setAlpha(alpha);
        c2.setAlpha(alpha);

        QLinearGradient cardGrad(cardRect.topLeft(), cardRect.bottomRight());
        cardGrad.setColorAt(0.0, c1);
        cardGrad.setColorAt(1.0, c2);

        QPainterPath path;
        path.addRoundedRect(cardRect, 14, 14);
        p.fillPath(path, cardGrad);

        if (g_cfg.enableBlurGlass) {
            QLinearGradient glassShine(cardRect.topLeft(), QPointF(cardRect.left(), cardRect.top() + cardRect.height() * 0.45));
            glassShine.setColorAt(0.0, QColor(255, 255, 255, static_cast<int>(28 + hoverProgress * 20)));
            glassShine.setColorAt(1.0, QColor(255, 255, 255, 0));
            p.fillPath(path, glassShine);
        }

        QColor borderColor;
        int borderAlpha = static_cast<int>(150 + hoverProgress * 105);
        if (cardData.pinned) {
            borderColor = QColor(251, 191, 36, borderAlpha);
        } else if (g_cfg.isMaleTheme) {
            borderColor = QColor(56, 189, 248, borderAlpha);
        } else {
            borderColor = QColor(244, 114, 182, borderAlpha);
        }

        p.setPen(QPen(borderColor, 1.6 + hoverProgress * 0.7));
        p.drawPath(path);
    }
};

// ============================================================================
// 2. ДРЕВОВИДНАЯ ИЕРАРХИЯ ДИРЕКТОРИЙ В ТРЕКЕРЕ ПРОГРЕССА
// ============================================================================
struct TrackerDirNode {
    std::string name;
    std::string fullSectionPath;
    int directCards = 0;
    int subtreeCards = 0;
    std::vector<TrackerDirNode> children;
};

class TrackerDirNodeWidget : public QWidget {
private:
    QWidget* childrenContainer = nullptr;
    NeonButton* toggleChildrenBtn = nullptr;

public:
    TrackerDirNodeWidget(const TrackerDirNode& node,
                         int depth,
                         int maxCardsForFull,
                         std::function<void(std::string)> onAddNoteInDir,
                         QWidget* parent = nullptr)
    : QWidget(parent)
    {
        QVBoxLayout* rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(depth > 0 ? 22 : 0, 3, 0, 3);
        rootLayout->setSpacing(6);

        QColor nodeColor = getModernColorForName(node.name);

        QHBoxLayout* row = new QHBoxLayout();
        row->setSpacing(10);

        QString branchPrefix = (depth == 0) ? "📁  " : "├─ 📁  ";
        QLabel* nameLbl = new QLabel(branchPrefix + QString::fromStdString(node.name), this);
        QFont nf = font();
        nf.setPointSize(g_cfg.uiFontSize);
        nf.setBold(depth == 0 || !node.children.empty());
        nameLbl->setFont(nf);
        nameLbl->setWordWrap(true);
        nameLbl->setMinimumWidth(160);
        QPalette pal = nameLbl->palette();
        pal.setColor(QPalette::WindowText, nodeColor.lighter(115));
        nameLbl->setPalette(pal);

        QLabel* countLbl = new QLabel(
            QString("Карточек: %1 / %2").arg(node.subtreeCards).arg(maxCardsForFull),
                                      this
        );

        int permille = std::clamp(
            static_cast<int>(std::round(static_cast<double>(node.subtreeCards) * 1000.0 / std::max(1, maxCardsForFull))),
                                  0, 1000
        );

        RoundedProgressBar* bar = new RoundedProgressBar(this);
        bar->setRange(0, 1000);
        bar->setValue(permille);
        bar->setBarColor(nodeColor);

        row->addWidget(nameLbl, 2);
        row->addWidget(countLbl);
        row->addWidget(bar, 2);

        if (!node.children.empty()) {
            size_t chCount = node.children.size();
            toggleChildrenBtn = new NeonButton(QString("▲ Подпапки (%1)").arg(chCount), nodeColor, this);
            toggleChildrenBtn->setFixedHeight(30);
            row->addWidget(toggleChildrenBtn);
        }

        NeonButton* addHereBtn = new NeonButton("+ Заметка", QColor("#4ade80"), this);
        addHereBtn->setFixedHeight(30);
        addHereBtn->setToolTip("Создать карточку в папке «" + QString::fromStdString(node.fullSectionPath) + "»");
        row->addWidget(addHereBtn);

        rootLayout->addLayout(row);

        if (!node.children.empty()) {
            childrenContainer = new QWidget(this);
            QVBoxLayout* chLayout = new QVBoxLayout(childrenContainer);
            chLayout->setContentsMargins(10, 2, 0, 2);
            chLayout->setSpacing(4);

            for (const auto& child : node.children) {
                chLayout->addWidget(new TrackerDirNodeWidget(
                    child, depth + 1, maxCardsForFull, onAddNoteInDir, childrenContainer
                ));
            }
            rootLayout->addWidget(childrenContainer);

            size_t chCount = node.children.size();
            connect(toggleChildrenBtn, &QPushButton::clicked, [this, chCount]() {
                bool vis = !childrenContainer->isVisible();
                childrenContainer->setVisible(vis);
                toggleChildrenBtn->setText(vis
                ? QString("▲ Подпапки (%1)").arg(chCount)
                : QString("▼ Подпапки (%1)").arg(chCount));
            });
        }

        std::string secPath = node.fullSectionPath;
        connect(addHereBtn, &QPushButton::clicked, [onAddNoteInDir, secPath]() {
            if (onAddNoteInDir) onAddNoteInDir(secPath);
        });
    }
};

class TrackerChapterCard : public QFrame {
private:
    QColor accentColor;
    QWidget* treeContainer = nullptr;
    NeonButton* expandBtn = nullptr;

public:
    TrackerChapterCard(const std::string& chapterTitle,
                       int startedDirs,
                       int totalDirs,
                       int totalChapterCards,
                       double weightShare,
                       int percentPermille,
                       const std::vector<TrackerDirNode>& rootNodes,
                       std::function<void(std::string)> onAddNoteInSection,
                       QWidget* parent = nullptr)
    : QFrame(parent), accentColor(getModernColorForName(chapterTitle))
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(24, 18, 24, 20);
        layout->setSpacing(12);

        QHBoxLayout* topRow = new QHBoxLayout();
        topRow->setSpacing(12);

        QLabel* titleLbl = new QLabel(QString::fromStdString("📘  " + chapterTitle), this);
        QFont tf = font();
        tf.setPointSize(g_cfg.uiFontSize + 2);
        tf.setBold(true);
        titleLbl->setFont(tf);
        titleLbl->setWordWrap(true);
        titleLbl->setContentsMargins(4, 2, 4, 2);
        QPalette tPal = titleLbl->palette();
        tPal.setColor(QPalette::WindowText, accentColor);
        titleLbl->setPalette(tPal);

        QString statText = QString("Директорий: %1 (активных: %2)   •   Карточек: %3 / %4   •   Вес: %5%")
        .arg(totalDirs)
        .arg(startedDirs)
        .arg(totalChapterCards)
        .arg(g_cfg.trackerCardsForFull)
        .arg(weightShare, 0, 'f', 1);
        QLabel* statLbl = new QLabel(statText, this);
        statLbl->setWordWrap(true);
        statLbl->setContentsMargins(6, 2, 6, 2);

        expandBtn = new NeonButton(QString("▼ Дерево папок (%1)").arg(totalDirs), accentColor, this);
        NeonButton* addBtn = new NeonButton("+ Заметка", QColor("#4ade80"), this);

        topRow->addWidget(titleLbl, 1);
        topRow->addWidget(statLbl);
        topRow->addWidget(expandBtn);
        topRow->addWidget(addBtn);

        RoundedProgressBar* mainBar = new RoundedProgressBar(this);
        mainBar->setRange(0, 1000);
        mainBar->setValue(percentPermille);
        mainBar->setBarColor(accentColor);

        treeContainer = new QWidget(this);
        QVBoxLayout* treeLayout = new QVBoxLayout(treeContainer);
        treeLayout->setContentsMargins(12, 10, 12, 6);
        treeLayout->setSpacing(8);

        for (const auto& rootNode : rootNodes) {
            treeLayout->addWidget(new TrackerDirNodeWidget(
                rootNode, 0, g_cfg.trackerCardsForFull, onAddNoteInSection, treeContainer
            ));
        }
        treeContainer->setVisible(false);

        layout->addLayout(topRow);
        layout->addWidget(mainBar);
        layout->addWidget(treeContainer);

        connect(expandBtn, &QPushButton::clicked, [this, totalDirs]() {
            bool open = !treeContainer->isVisible();
            treeContainer->setVisible(open);
            expandBtn->setText(open
            ? QString("▲ Свернуть дерево (%1)").arg(totalDirs)
            : QString("▼ Дерево папок (%1)").arg(totalDirs));
        });

        std::string defaultSec = rootNodes.empty() ? "Заметки" : rootNodes.front().fullSectionPath;
        connect(addBtn, &QPushButton::clicked, [onAddNoteInSection, defaultSec]() {
            if (onAddNoteInSection) onAddNoteInSection(defaultSec);
        });
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(2, 2, -2, -2);
        p.setBrush(g_cfg.bgTrackerCard);
        QColor bc = accentColor;
        bc.setAlpha(140);
        p.setPen(QPen(bc, 1.5));
        p.drawRoundedRect(r, 16, 16);
    }
};

// ============================================================================
// 3. ГЛАВНОЕ ОКНО ПРОГРАММЫ
//    - Панели выдвигаются строго ВНУТРЬ окна (не расширяя программу наружу)
//    - Ширина правой панели меняется только в Настройках
//    - Уменьшение окна до 1 блока карточки с автопереводом панелей в отдельные окна
//    - Немодальное окно «Подробнее» для параллельной работы с карточками
// ============================================================================
class MainWindow : public QMainWindow {
private:
    fs::path vaultRoot = u8path("./vault");
    DatabaseManager db{"database.sqlite", vaultRoot};
    QFileSystemWatcher* fsWatcher = nullptr;

    std::string currentChapter = "all";
    std::string currentSection = "all";
    int lastGridColumns = -1;
    std::vector<CardBlockWidget*> activeCardWidgets;

    UnifiedLeftPanel* leftVaultPanel = nullptr;
    QPointer<FloatingVaultWindow> floatingVaultWindow;
    QPushButton* toggleFileMgrBtn = nullptr;

    RightDrawerPanel* rightDrawerPanel = nullptr;
    QPointer<DetailsEditorDialog> floatingDetailsWindow;

    FloatingFormatMiniBar* floatingFormatBar = nullptr;

    QStackedWidget* centerStack = nullptr;
    QLabel* mainTitleLabel = nullptr;
    QLabel* currentChapterBadge = nullptr;
    QLabel* dockHotkeyHintLabel = nullptr;
    QHBoxLayout* sectionTabsLayout = nullptr;
    QLineEdit* searchInput = nullptr;

    QScrollArea* cardsScrollArea = nullptr;
    QWidget* cardsGridContainer = nullptr;
    QGridLayout* cardsGridLayout = nullptr;

    QVBoxLayout* trackerListLayout = nullptr;
    RoundedProgressBar* overallProgressBar = nullptr;
    QLabel* overallStatLabel = nullptr;

    NeonButton* switchViewBtn = nullptr;
    NeonButton* themeSwitchBtn = nullptr;

    QShortcut* scNewCard = nullptr;
    QShortcut* scToggleVault = nullptr;
    QShortcut* scToggleDrawer = nullptr;
    QShortcut* scFormatPopup = nullptr;
    QShortcut* scTracker = nullptr;
    QShortcut* scSettings = nullptr;
    QShortcut* scSearch = nullptr;

public:
    MainWindow() {
        setWindowTitle("Интерактивная база знаний и Трекер (Windows 11 / Linux C++17)");

        CfgBridge::loadExtraSettings();
        db.syncDirectoryWithDatabase();

        setupUI();
        updateMinimumWindowSizeToOneBlock();
        CfgBridge::restoreWindowRect("mainWindowGeometry", this, QSize(1440, 880));

        setupShortcuts();
        setupFileSystemWatcher();
        applyModernThemePalette();
        refreshAllViews();
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QLinearGradient bgGrad(0, 0, width(), height());
        bgGrad.setColorAt(0.0, g_cfg.bgWindowTop);
        bgGrad.setColorAt(0.5, g_cfg.bgWindowBottom);
        bgGrad.setColorAt(1.0, g_cfg.bgWindowTop.darker(120));
        p.fillRect(rect(), bgGrad);

        QRadialGradient glow(width() * 0.5, height() * 0.18, width() * 0.55);
        if (g_cfg.isMaleTheme) {
            glow.setColorAt(0.0, QColor(56, 189, 248, 42));
        } else {
            glow.setColorAt(0.0, QColor(244, 114, 182, 42));
        }
        glow.setColorAt(1.0, QColor(0, 0, 0, 0));
        p.fillRect(rect(), glow);
    }

    void moveEvent(QMoveEvent* event) override {
        QMainWindow::moveEvent(event);
        if (isVisible()) {
            CfgBridge::saveWindowRect("mainWindowGeometry", this);
        }
    }

    void resizeEvent(QResizeEvent* event) override {
        QMainWindow::resizeEvent(event);
        if (isVisible()) {
            CfgBridge::saveWindowRect("mainWindowGeometry", this);
        }

        // Если пользователь сжал главное окно мышью до малого режима —
        // автоматически переводим открытые внутренние панели в отдельные плавающие окна,
        // чтобы они не мешали уменьшать программу до размера 1 карточки.
        if (leftVaultPanel && leftVaultPanel->isVisible() && isTooNarrowForInternalLeftPanel()) {
            leftVaultPanel->setVisible(false);
            toggleFileMgrBtn->setText("▶");
            openOrRaiseFloatingVaultWindow();
        }
        if (rightDrawerPanel && rightDrawerPanel->isVisible() && isTooNarrowForInternalRightPanel()) {
            long long cid = rightDrawerPanel->currentCardId();
            rightDrawerPanel->setVisible(false);
            for (const auto& c : db.getAllCards()) {
                if (c.id == cid) {
                    openOrUpdateFloatingDetailsWindow(c);
                    break;
                }
            }
        }

        if (mainTitleLabel) {
            mainTitleLabel->setVisible(width() >= 680);
        }

        checkAndReflowGrid();
    }

    void closeEvent(QCloseEvent* event) override {
        CfgBridge::saveWindowRect("mainWindowGeometry", this);
        if (floatingDetailsWindow) {
            floatingDetailsWindow->close();
        }
        if (floatingVaultWindow) {
            floatingVaultWindow->close();
        }
        QMainWindow::closeEvent(event);
    }

    bool eventFilter(QObject* watched, QEvent* event) override {
        if (cardsScrollArea && watched == cardsScrollArea->viewport() && event->type() == QEvent::Resize) {
            checkAndReflowGrid();
        }
        return QMainWindow::eventFilter(watched, event);
    }

private:
    // Минимальный размер главного окна равен размеру одного блока карточки
    void updateMinimumWindowSizeToOneBlock() {
        int cardW = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int minW  = std::max(320, cardW + 56);
        int minH  = 430;
        setMinimumSize(minW, minH);
    }

    // Проверка: помещается ли внутри окна левая панель + хотя бы 1 карточка
    bool isTooNarrowForInternalLeftPanel() const {
        int cardW = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int rightW = (rightDrawerPanel && rightDrawerPanel->isVisible()) ? rightDrawerPanel->width() : 0;
        return width() < (cardW + 315 + rightW + 64);
    }

    // Проверка: помещается ли внутри окна правая панель + хотя бы 1 карточка
    bool isTooNarrowForInternalRightPanel() const {
        int cardW = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int drawerW = std::clamp(g_cfg.rightDrawerWidth, 240, 950);
        int leftW = (leftVaultPanel && leftVaultPanel->isVisible()) ? leftVaultPanel->width() : 0;
        return width() < (cardW + drawerW + leftW + 64);
    }

    int calculateGridColumns() const {
        int cardW = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int spacing = cardsGridLayout ? cardsGridLayout->horizontalSpacing() : 18;
        if (spacing < 0) spacing = 18;

        int marginsLR = 24;
        if (cardsGridLayout) {
            QMargins m = cardsGridLayout->contentsMargins();
            marginsLR = m.left() + m.right();
        }

        int viewportW = (cardsScrollArea && cardsScrollArea->viewport()->width() > 100)
        ? cardsScrollArea->viewport()->width()
        : (width()
        - (leftVaultPanel && leftVaultPanel->isVisible() ? leftVaultPanel->width() : 0)
        - (rightDrawerPanel && rightDrawerPanel->isVisible() ? rightDrawerPanel->width() : 0)
        - 60);

        int usableW = viewportW - marginsLR - 8;
        return std::max(1, (usableW + spacing) / (cardW + spacing));
    }

    void relayoutCardsGrid(int columns) {
        if (!cardsGridLayout) return;
        lastGridColumns = std::max(1, columns);

        for (CardBlockWidget* w : activeCardWidgets) {
            cardsGridLayout->removeWidget(w);
        }

        int maxOldRows = static_cast<int>(activeCardWidgets.size()) + 4;
        for (int r = 0; r <= maxOldRows; ++r) {
            cardsGridLayout->setRowStretch(r, 0);
        }
        for (int c = 0; c < 12; ++c) {
            cardsGridLayout->setColumnStretch(c, 0);
        }

        for (size_t i = 0; i < activeCardWidgets.size(); ++i) {
            int row = static_cast<int>(i) / lastGridColumns;
            int col = static_cast<int>(i) % lastGridColumns;
            cardsGridLayout->addWidget(activeCardWidgets[i], row, col, Qt::AlignTop | Qt::AlignHCenter);
        }

        int bottomStretchRow = static_cast<int>(activeCardWidgets.size()) / lastGridColumns + 1;
        cardsGridLayout->setRowStretch(bottomStretchRow, 1);
        cardsGridLayout->invalidate();
        if (cardsGridContainer) {
            cardsGridContainer->adjustSize();
        }
    }

    void checkAndReflowGrid() {
        if (!cardsGridLayout || activeCardWidgets.empty()) return;
        int newCols = calculateGridColumns();
        if (newCols != lastGridColumns) {
            relayoutCardsGrid(newCols);
        }
    }

    void scheduleReflowAfterPanelToggle() {
        checkAndReflowGrid();
        QTimer::singleShot(0, this, [this]() { checkAndReflowGrid(); });
        QTimer::singleShot(35, this, [this]() { checkAndReflowGrid(); });
    }

    void updateCardDetailsDotsInPlace() {
        for (CardBlockWidget* w : activeCardWidgets) {
            if (w) w->refreshDetailsDotFromDb();
        }
    }

    QString formatCurrentBreadcrumb() const {
        QString ch = (currentChapter == "all") ? "Все главы" : QString::fromStdString(currentChapter);
        if (currentSection == "all") {
            return "Глава: " + ch;
        }
        QString sec = QString::fromStdString(currentSection);
        sec.replace("/", "  ▸  ");
        return "Глава: " + ch + "  ▸  " + sec;
    }

    void updateHotkeyHintText() {
        if (!dockHotkeyHintLabel) return;
        dockHotkeyHintLabel->setText(
            "✨ Док-панель у курсора: [ " + g_cfg.hkFormatPopup + " ]   "
            "(блок кода: [ " + CfgBridge::s_hkInsertCode + " ])"
        );
        QPalette p = dockHotkeyHintLabel->palette();
        p.setColor(QPalette::WindowText, g_cfg.isMaleTheme ? QColor("#38bdf8") : QColor("#f472b6"));
        dockHotkeyHintLabel->setPalette(p);
    }

    void setupShortcuts() {
        if (!scNewCard) {
            scNewCard      = new QShortcut(this);
            scToggleVault  = new QShortcut(this);
            scToggleDrawer = new QShortcut(this);
            scFormatPopup  = new QShortcut(this);
            scTracker      = new QShortcut(this);
            scSettings     = new QShortcut(this);
            scSearch       = new QShortcut(this);

            scNewCard->setAutoRepeat(false);
            scToggleVault->setAutoRepeat(false);
            scToggleDrawer->setAutoRepeat(false);
            scFormatPopup->setAutoRepeat(false);
            scTracker->setAutoRepeat(false);
            scSettings->setAutoRepeat(false);
            scSearch->setAutoRepeat(false);

            connect(scNewCard,      &QShortcut::activated, [this]() { openCreateCardModal(); });
            connect(scToggleVault,  &QShortcut::activated, [this]() { toggleVaultPanel(); });
            connect(scToggleDrawer, &QShortcut::activated, [this]() { toggleRightDrawerHotkey(); });
            connect(scFormatPopup,  &QShortcut::activated, [this]() {
                if (floatingFormatBar) floatingFormatBar->toggleAtCursor();
            });
                connect(scTracker,      &QShortcut::activated, [this]() { toggleTrackerView(); });
                connect(scSettings,     &QShortcut::activated, [this]() { openSettingsModal(); });
                connect(scSearch,       &QShortcut::activated, [this]() {
                    centerStack->setCurrentIndex(0);
                    searchInput->setFocus();
                    searchInput->selectAll();
                });
        }

        scNewCard->setKey(QKeySequence(g_cfg.hkNewCard));
        scToggleVault->setKey(QKeySequence(g_cfg.hkToggleVault));
        scToggleDrawer->setKey(QKeySequence(g_cfg.hkToggleDrawer));
        scFormatPopup->setKey(QKeySequence(g_cfg.hkFormatPopup));
        scTracker->setKey(QKeySequence(g_cfg.hkTracker));
        scSettings->setKey(QKeySequence(g_cfg.hkSettings));
        scSearch->setKey(QKeySequence(g_cfg.hkFocusSearch));

        updateHotkeyHintText();
    }

    void openOrUpdateFloatingDetailsWindow(const Card& c) {
        if (!floatingDetailsWindow) {
            floatingDetailsWindow = new DetailsEditorDialog(
                c,
                db,
                this,
                [this]() { updateCardDetailsDotsInPlace(); }
            );
        } else {
            floatingDetailsWindow->loadCard(c);
        }
        floatingDetailsWindow->show();
        floatingDetailsWindow->raise();
        floatingDetailsWindow->activateWindow();
    }

    void openOrRaiseFloatingVaultWindow() {
        if (!floatingVaultWindow) {
            floatingVaultWindow = new FloatingVaultWindow(
                vaultRoot,
                db,
                [this]() {
                    currentChapter = "all";
                    currentSection = "all";
                    centerStack->setCurrentIndex(0);
                    refreshAllViews();
                },
                [this](std::string ch, std::string sec) {
                    currentChapter = ch;
                    currentSection = sec;
                    centerStack->setCurrentIndex(0);
                    currentChapterBadge->setText(formatCurrentBreadcrumb());
                    refreshSectionTabs();
                    renderCardsGrid();
                },
                [this](Card c) { openDetailsForCard(c); },
                                                          [this]() {
                                                              updateWatchedPaths();
                                                              refreshAllViews();
                                                          },
                                                          this
            );
        }
        floatingVaultWindow->refreshTree();
        floatingVaultWindow->show();
        floatingVaultWindow->raise();
        floatingVaultWindow->activateWindow();
    }

    void toggleRightDrawerHotkey() {
        // Если уже открыто плавающее окно «Подробнее» — закрываем его
        if (floatingDetailsWindow && floatingDetailsWindow->isVisible()) {
            floatingDetailsWindow->close();
            return;
        }
        // Если открыта внутренняя правая панель — скрываем её внутрь
        if (rightDrawerPanel->isVisible()) {
            const QSize savedWinSize = size();
            rightDrawerPanel->setVisible(false);
            resize(savedWinSize);
            scheduleReflowAfterPanelToggle();
            return;
        }

        Card targetCard;
        if (rightDrawerPanel->hasCard()) {
            for (const auto& c : db.getAllCards()) {
                if (c.id == rightDrawerPanel->currentCardId()) {
                    targetCard = c;
                    break;
                }
            }
        }
        if (targetCard.id == 0) {
            auto allCards = db.getAllCards();
            if (!allCards.empty()) targetCard = allCards.front();
        }
        if (targetCard.id != 0) {
            openDetailsForCard(targetCard);
        }
    }

    QTimer* fsSyncTimer = nullptr;

    void setupFileSystemWatcher() {
        fsWatcher = new QFileSystemWatcher(this);
        updateWatchedPaths();

        fsSyncTimer = new QTimer(this);
        fsSyncTimer->setSingleShot(true);
        fsSyncTimer->setInterval(700);
        connect(fsSyncTimer, &QTimer::timeout, [this]() {
            if (g_ignoreFsWatcher) {
                fsSyncTimer->start();
                return;
            }
            if (db.syncDirectoryWithDatabase()) {
                updateWatchedPaths();
                refreshAllViews();
            }
        });

        connect(fsWatcher, &QFileSystemWatcher::directoryChanged, [this](const QString&) {
            if (g_ignoreFsWatcher) return;
            fsSyncTimer->start();
        });
        connect(fsWatcher, &QFileSystemWatcher::fileChanged, [this](const QString& p) {
            std::error_code ec;
            if (fs::exists(u8path(p.toStdString()), ec)) {
                if (!fsWatcher->files().contains(p)) {
                    fsWatcher->addPath(p);
                }
            }
            if (g_ignoreFsWatcher) return;
            fsSyncTimer->start();
        });
    }

    void updateWatchedPaths() {
        std::error_code ec;
        if (!fs::exists(vaultRoot, ec)) return;
        QStringList paths;
        paths << QString::fromStdString(pathToUtf8(vaultRoot));
        for (const auto& entry : fs::recursive_directory_iterator(vaultRoot, fs::directory_options::skip_permission_denied, ec)) {
            std::string name = filenameToUtf8(entry.path());
            if (!name.empty() && name[0] == '.') continue;
            if (entry.is_directory(ec)) {
                paths << QString::fromStdString(pathToUtf8(entry.path()));
            } else if (entry.is_regular_file(ec) && DatabaseManager::isNoteFile(entry.path())) {
                paths << QString::fromStdString(pathToUtf8(entry.path()));
            }
        }
        QStringList already = fsWatcher->directories() + fsWatcher->files();
        for (const QString& p : paths) {
            if (!already.contains(p)) fsWatcher->addPath(p);
        }
    }

    void setupUI() {
        QWidget* central = new QWidget(this);
        // Ignored позволяет сжимать окно до 1 блока карточки и выдвигать панели строго ВНУТРЬ окна!
        central->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        setCentralWidget(central);

        QHBoxLayout* rootLayout = new QHBoxLayout(central);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);

        QFont boldF = font();
        boldF.setBold(true);

        leftVaultPanel = new UnifiedLeftPanel(
            vaultRoot,
            db,
            [this]() {
                currentChapter = "all";
                currentSection = "all";
                centerStack->setCurrentIndex(0);
                refreshAllViews();
            },
            [this](std::string ch, std::string sec) {
                currentChapter = ch;
                currentSection = sec;
                centerStack->setCurrentIndex(0);
                currentChapterBadge->setText(formatCurrentBreadcrumb());
                refreshSectionTabs();
                renderCardsGrid();
            },
            [this](Card c) { openDetailsForCard(c); },
                                              [this]() {
                                                  updateWatchedPaths();
                                                  refreshAllViews();
                                              },
                                              this,
                                              false
        );
        leftVaultPanel->setVisible(false);

        QWidget* arrowBar = new QWidget(this);
        arrowBar->setFixedWidth(26);
        QVBoxLayout* arrowLayout = new QVBoxLayout(arrowBar);
        arrowLayout->setContentsMargins(2, 0, 2, 0);

        toggleFileMgrBtn = new QPushButton("▶", this);
        toggleFileMgrBtn->setFixedSize(22, 68);
        toggleFileMgrBtn->setToolTip("Открыть / скрыть проводник (Ctrl+B)");

        arrowLayout->addStretch(1);
        arrowLayout->addWidget(toggleFileMgrBtn);
        arrowLayout->addStretch(1);

        QWidget* mainArea = new QWidget(this);
        mainArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
        QVBoxLayout* mainLayout = new QVBoxLayout(mainArea);
        mainLayout->setContentsMargins(16, 14, 16, 12);
        mainLayout->setSpacing(8);

        QHBoxLayout* topButtonsBar = new QHBoxLayout();
        topButtonsBar->setContentsMargins(2, 2, 2, 2);
        topButtonsBar->setSpacing(8);
        NeonButton* burgerBtn   = new NeonButton("☰ Главы", QColor("#38bdf8"), this);
        NeonButton* settingsBtn = new NeonButton("⚙ Настройки", QColor("#a78bfa"), this);
        NeonButton* addCardBtn  = new NeonButton("+ Карточка", QColor("#4ade80"), this);

        mainTitleLabel = new QLabel("Интерактивная база знаний", this);
        QFont hFont = font();
        hFont.setPointSize(g_cfg.uiFontSize + 3);
        hFont.setBold(true);
        mainTitleLabel->setFont(hFont);
        mainTitleLabel->setAlignment(Qt::AlignCenter);
        mainTitleLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

        topButtonsBar->addWidget(burgerBtn);
        topButtonsBar->addWidget(mainTitleLabel, 1);
        topButtonsBar->addWidget(settingsBtn);
        topButtonsBar->addWidget(addCardBtn);

        centerStack = new QStackedWidget(this);
        centerStack->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);

        // --- Вкладка 0: База знаний ---
        QWidget* kbPage = new QWidget(this);
        QVBoxLayout* kbLayout = new QVBoxLayout(kbPage);
        kbLayout->setContentsMargins(2, 2, 2, 2);
        kbLayout->setSpacing(6);

        currentChapterBadge = new QLabel("Глава: Все главы", this);
        currentChapterBadge->setAlignment(Qt::AlignCenter);
        currentChapterBadge->setFont(boldF);
        currentChapterBadge->setWordWrap(true);
        currentChapterBadge->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);

        dockHotkeyHintLabel = new QLabel(this);
        dockHotkeyHintLabel->setAlignment(Qt::AlignCenter);
        dockHotkeyHintLabel->setFont(boldF);
        dockHotkeyHintLabel->setWordWrap(true);
        dockHotkeyHintLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        updateHotkeyHintText();

        sectionTabsLayout = new QHBoxLayout();
        sectionTabsLayout->setContentsMargins(2, 2, 2, 2);
        sectionTabsLayout->setSpacing(6);

        searchInput = new QLineEdit(this);
        searchInput->setPlaceholderText("Поиск по командам, заголовкам и описанию...");
        searchInput->setMinimumHeight(30);
        NeonInputFrame* searchBoxFrame = new NeonInputFrame(searchInput, QColor("#38bdf8"), this);

        cardsScrollArea = new QScrollArea(this);
        cardsScrollArea->setWidgetResizable(true);
        cardsScrollArea->setFrameShape(QFrame::NoFrame);
        cardsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        cardsScrollArea->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
        cardsScrollArea->viewport()->setAutoFillBackground(false);
        cardsScrollArea->viewport()->installEventFilter(this);

        cardsGridContainer = new QWidget(this);
        cardsGridContainer->setAutoFillBackground(false);
        cardsGridLayout = new QGridLayout(cardsGridContainer);
        cardsGridLayout->setSpacing(18);
        cardsGridLayout->setContentsMargins(8, 10, 8, 16);
        cardsScrollArea->setWidget(cardsGridContainer);

        kbLayout->addWidget(currentChapterBadge);
        kbLayout->addWidget(dockHotkeyHintLabel);
        kbLayout->addLayout(sectionTabsLayout);
        kbLayout->addWidget(searchBoxFrame);
        kbLayout->addWidget(cardsScrollArea, 1);

        // --- Вкладка 1: Трекер прогресса ---
        QWidget* trackerPage = new QWidget(this);
        QVBoxLayout* trLayout = new QVBoxLayout(trackerPage);
        trLayout->setContentsMargins(8, 6, 8, 6);
        trLayout->setSpacing(12);

        QFrame* overallHeaderCard = new QFrame(this);
        QVBoxLayout* ohLayout = new QVBoxLayout(overallHeaderCard);
        ohLayout->setContentsMargins(18, 14, 18, 16);
        ohLayout->setSpacing(8);

        overallStatLabel = new QLabel("Общий прогресс по базе знаний:", this);
        overallStatLabel->setFont(boldF);
        overallStatLabel->setWordWrap(true);

        overallProgressBar = new RoundedProgressBar(this);
        overallProgressBar->setRange(0, 1000);
        overallProgressBar->setBarColor(QColor("#38bdf8"));

        ohLayout->addWidget(overallStatLabel);
        ohLayout->addWidget(overallProgressBar);

        QScrollArea* trScroll = new QScrollArea(this);
        trScroll->setWidgetResizable(true);
        trScroll->setFrameShape(QFrame::NoFrame);
        trScroll->viewport()->setAutoFillBackground(false);

        QWidget* trContainer = new QWidget(this);
        trContainer->setAutoFillBackground(false);
        trackerListLayout = new QVBoxLayout(trContainer);
        trackerListLayout->setContentsMargins(4, 6, 8, 14);
        trackerListLayout->setSpacing(12);
        trScroll->setWidget(trContainer);

        trLayout->addWidget(overallHeaderCard);
        trLayout->addWidget(trScroll, 1);

        centerStack->addWidget(kbPage);
        centerStack->addWidget(trackerPage);

        QHBoxLayout* bottomBar = new QHBoxLayout();
        bottomBar->setContentsMargins(4, 2, 4, 2);
        switchViewBtn  = new NeonButton("📊 Трекер прогресса", QColor("#38bdf8"), this);
        themeSwitchBtn = new NeonButton(g_cfg.isMaleTheme ? "👨 Тема: Мужская" : "👩 Тема: Женская", QColor("#f472b6"), this);

        bottomBar->addStretch(1);
        bottomBar->addWidget(switchViewBtn);
        bottomBar->addStretch(1);
        bottomBar->addWidget(themeSwitchBtn);

        mainLayout->addLayout(topButtonsBar);
        mainLayout->addWidget(centerStack, 1);
        mainLayout->addLayout(bottomBar);

        rightDrawerPanel = new RightDrawerPanel(db, [this]() {
            updateCardDetailsDotsInPlace();
            scheduleReflowAfterPanelToggle();
        }, this);
        rightDrawerPanel->setVisible(false);

        // Вкладываем все панели в единый горизонтальный макет (без ручного QSplitter):
        // левая и правая панели выдвигаются строго ВНУТРЬ главного окна!
        rootLayout->addWidget(leftVaultPanel);
        rootLayout->addWidget(arrowBar);
        rootLayout->addWidget(mainArea, 1);
        rootLayout->addWidget(rightDrawerPanel);

        floatingFormatBar = new FloatingFormatMiniBar(this);
        floatingFormatBar->hide();

        connect(burgerBtn,        &QPushButton::clicked, [this]() { toggleVaultPanel(); });
        connect(toggleFileMgrBtn, &QPushButton::clicked, [this]() { toggleVaultPanel(); });
        connect(settingsBtn,      &QPushButton::clicked, [this]() { openSettingsModal(); });
        connect(addCardBtn,       &QPushButton::clicked, [this]() { openCreateCardModal(); });
        connect(searchInput,      &QLineEdit::textChanged, [this]() { renderCardsGrid(); });
        connect(switchViewBtn,    &QPushButton::clicked, [this]() { toggleTrackerView(); });

        connect(themeSwitchBtn, &QPushButton::clicked, [this]() {
            g_cfg.isMaleTheme = !g_cfg.isMaleTheme;
            themeSwitchBtn->setText(g_cfg.isMaleTheme ? "👨 Тема: Мужская" : "👩 Тема: Женская");
            overallProgressBar->setBarColor(g_cfg.isMaleTheme ? QColor("#38bdf8") : QColor("#f472b6"));
            updateHotkeyHintText();
            db.saveConfig(g_cfg);
            applyModernThemePalette();
            leftVaultPanel->update();
            rightDrawerPanel->update();
            if (floatingFormatBar) floatingFormatBar->update();
            update();
            renderCardsGrid();
        });
    }

    // Открытие подробного описания:
    // - Если выбран режим «Боковая панель» И окно достаточно широкое -> выдвигаем правую панель ВНУТРЬ окна
    // - Если выбран режим «Отдельное окно» ИЛИ главное окно сжато в малый режим -> открываем немодальное окно
    void openDetailsForCard(Card c) {
        if (g_cfg.useRightDrawer && !isTooNarrowForInternalRightPanel()) {
            if (floatingDetailsWindow && floatingDetailsWindow->isVisible()) {
                floatingDetailsWindow->close();
            }
            const QSize savedWinSize = size();
            rightDrawerPanel->applyConfiguredWidth();
            rightDrawerPanel->loadCard(c);
            rightDrawerPanel->setVisible(true);
            resize(savedWinSize); // Гарантируем, что внешние габариты окна не изменились ни на пиксель
            scheduleReflowAfterPanelToggle();
        } else {
            if (rightDrawerPanel->isVisible()) {
                const QSize savedWinSize = size();
                rightDrawerPanel->setVisible(false);
                resize(savedWinSize);
                scheduleReflowAfterPanelToggle();
            }
            openOrUpdateFloatingDetailsWindow(c);
        }
    }

    // Выдвижение левой панели:
    // - В обычном режиме выдвигается строго внутрь окна
    // - В малом режиме (узкое окно) открывается как отдельное плавающее окно
    void toggleVaultPanel() {
        if (floatingVaultWindow && floatingVaultWindow->isVisible()) {
            floatingVaultWindow->close();
            toggleFileMgrBtn->setText("▶");
            return;
        }

        if (leftVaultPanel->isVisible()) {
            const QSize savedWinSize = size();
            leftVaultPanel->setVisible(false);
            toggleFileMgrBtn->setText("▶");
            resize(savedWinSize);
            scheduleReflowAfterPanelToggle();
            return;
        }

        if (isTooNarrowForInternalLeftPanel()) {
            openOrRaiseFloatingVaultWindow();
            return;
        }

        const QSize savedWinSize = size();
        leftVaultPanel->setVisible(true);
        toggleFileMgrBtn->setText("◀");
        resize(savedWinSize);
        scheduleReflowAfterPanelToggle();
    }

    void toggleTrackerView() {
        if (centerStack->currentIndex() == 0) {
            centerStack->setCurrentIndex(1);
            if (floatingFormatBar) floatingFormatBar->hide();
            switchViewBtn->setText("📚 База знаний");
            renderWeightedTracker();
        } else {
            centerStack->setCurrentIndex(0);
            switchViewBtn->setText("📊 Трекер прогресса");
            refreshAllViews();
        }
    }

    void openCreateCardModal() {
        Card c;
        c.chapter = (currentChapter == "all") ? "Общее" : currentChapter;
        c.section = (currentSection == "all") ? "Заметки" : currentSection;
        CardEditorDialog dlg(c, true, db, this);
        if (dlg.exec() == QDialog::Accepted) {
            updateWatchedPaths();
            refreshAllViews();
        }
    }

    void openSettingsModal() {
        SettingsDialog dlg(db, [this]() {
            setupShortcuts();
            updateMinimumWindowSizeToOneBlock();
            applyModernThemePalette();

            // Мгновенно применяем новую ширину правой панели из настроек
            rightDrawerPanel->applyConfiguredWidth();

            if (rightDrawerPanel->isVisible() && (!g_cfg.useRightDrawer || isTooNarrowForInternalRightPanel())) {
                long long cid = rightDrawerPanel->currentCardId();
                rightDrawerPanel->setVisible(false);
                for (const auto& c : db.getAllCards()) {
                    if (c.id == cid) {
                        openOrUpdateFloatingDetailsWindow(c);
                        break;
                    }
                }
            }

            leftVaultPanel->update();
            rightDrawerPanel->update();
            if (floatingFormatBar) floatingFormatBar->update();
            update();
            refreshAllViews();
            scheduleReflowAfterPanelToggle();
        }, this);
        dlg.exec();
    }

    void refreshAllViews() {
        leftVaultPanel->refreshTree();
        if (floatingVaultWindow) {
            floatingVaultWindow->refreshTree();
        }
        currentChapterBadge->setText(formatCurrentBreadcrumb());
        updateHotkeyHintText();
        refreshSectionTabs();
        renderCardsGrid();
        if (centerStack->currentIndex() == 1) {
            renderWeightedTracker();
        }
    }

    void refreshSectionTabs() {
        QLayoutItem* child;
        while ((child = sectionTabsLayout->takeAt(0)) != nullptr) {
            if (child->widget()) delete child->widget();
            delete child;
        }

        auto allCards = db.getAllCards();
        std::vector<std::string> sections = {"all"};

        for (const auto& c : allCards) {
            if (currentChapter != "all" && c.chapter != currentChapter) continue;
            if (c.section.empty()) continue;

            if (std::find(sections.begin(), sections.end(), c.section) == sections.end()) {
                sections.push_back(c.section);
            }
        }

        sectionTabsLayout->addStretch(1);
        for (const auto& sec : sections) {
            QColor tabColor = (sec == "all") ? QColor("#38bdf8") : getModernColorForName(sec);
            QString btnText = (sec == "all") ? "Все разделы" : QString::fromStdString(sec).replace("/", " ▸ ");
            if (sec == currentSection) {
                btnText = "● " + btnText;
            }
            NeonButton* tabBtn = new NeonButton(btnText, tabColor, this);
            tabBtn->setMaximumWidth(280);
            connect(tabBtn, &QPushButton::clicked, [this, sec]() {
                currentSection = sec;
                currentChapterBadge->setText(formatCurrentBreadcrumb());
                refreshSectionTabs();
                renderCardsGrid();
            });
            sectionTabsLayout->addWidget(tabBtn);
        }
        sectionTabsLayout->addStretch(1);
    }

    void renderCardsGrid() {
        if (!cardsGridLayout) return;

        g_activeRichEditor = nullptr;
        activeCardWidgets.clear();

        QLayoutItem* item;
        while ((item = cardsGridLayout->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }

        auto allCards = db.getAllCards();
        std::string query = searchInput->text().toLower().toStdString();

        std::vector<Card> filtered;
        for (const auto& c : allCards) {
            if (currentChapter != "all" && c.chapter != currentChapter) continue;
            if (currentSection != "all" && c.section != currentSection) continue;

            if (!query.empty()) {
                std::string plainDesc = RichTextCodec::toPlainText(c.description);
                std::string plainExt  = RichTextCodec::toPlainText(c.extendedDesc);
                std::string haystack = QString::fromStdString(c.title + " " + c.code + " " + plainDesc + " " + plainExt).toLower().toStdString();
                if (haystack.find(query) == std::string::npos) continue;
            }
            filtered.push_back(c);
        }

        for (size_t i = 0; i < filtered.size(); ++i) {
            CardBlockWidget* block = new CardBlockWidget(
                filtered[i],
                db,
                [this](Card card) { openDetailsForCard(card); },
                                                         [this](Card card) {
                                                             card.pinned = !card.pinned;
                                                             db.saveCard(card, false);
                                                             renderCardsGrid();
                                                         },
                                                         [this](Card card) {
                                                             if (QMessageBox::question(this, "Удаление", "Удалить карточку «" + QString::fromStdString(card.title) + "»?") == QMessageBox::Yes) {
                                                                 db.deleteCard(card.id);
                                                                 refreshAllViews();
                                                             }
                                                         },
                                                         cardsGridContainer
            );
            activeCardWidgets.push_back(block);
        }

        relayoutCardsGrid(calculateGridColumns());
    }

    static int cardsToPermille(int cardCount) {
        int full = std::max(1, g_cfg.trackerCardsForFull);
        double ratio = static_cast<double>(cardCount) / static_cast<double>(full);
        return std::clamp(static_cast<int>(std::round(ratio * 1000.0)), 0, 1000);
    }

    struct RawTreeBuilderNode {
        std::string name;
        std::string fullPath;
        int directCards = 0;
        std::map<std::string, RawTreeBuilderNode> children;
    };

    static void insertSectionPathIntoTree(std::map<std::string, RawTreeBuilderNode>& rootMap,
                                          const std::string& sectionPath,
                                          int cardIncrement)
    {
        QString secStr = QString::fromStdString(sectionPath);
        QStringList parts = secStr.split(QRegularExpression("[/\\\\]"), Qt::SkipEmptyParts);
        if (parts.isEmpty()) return;

        std::map<std::string, RawTreeBuilderNode>* currentMap = &rootMap;
        std::string builtPath;

        for (int i = 0; i < parts.size(); ++i) {
            std::string part = parts[i].trimmed().toStdString();
            if (part.empty()) continue;

            if (!builtPath.empty()) builtPath += "/";
            builtPath += part;

            RawTreeBuilderNode& node = (*currentMap)[part];
            node.name = part;
            node.fullPath = builtPath;
            if (i == parts.size() - 1) {
                node.directCards += cardIncrement;
            }
            currentMap = &node.children;
        }
    }

    static TrackerDirNode finalizeTrackerTree(const RawTreeBuilderNode& raw, int& totalDirsOut, int& startedDirsOut, int& completedDirsOut) {
        TrackerDirNode out;
        out.name = raw.name;
        out.fullSectionPath = raw.fullPath;
        out.directCards = raw.directCards;
        out.subtreeCards = raw.directCards;

        for (const auto& [k, childRaw] : raw.children) {
            TrackerDirNode childNode = finalizeTrackerTree(childRaw, totalDirsOut, startedDirsOut, completedDirsOut);
            out.subtreeCards += childNode.subtreeCards;
            out.children.push_back(childNode);
        }

        std::sort(out.children.begin(), out.children.end(), [](const TrackerDirNode& a, const TrackerDirNode& b) {
            int ia = std::atoi(a.name.c_str());
            int ib = std::atoi(b.name.c_str());
            if (ia > 0 && ib > 0 && ia != ib) return ia < ib;
            return a.name < b.name;
        });

        totalDirsOut++;
        if (out.subtreeCards > 0) startedDirsOut++;
        if (out.subtreeCards >= g_cfg.trackerCardsForFull) completedDirsOut++;

        return out;
    }

    void renderWeightedTracker() {
        QLayoutItem* item;
        while ((item = trackerListLayout->takeAt(0)) != nullptr) {
            if (item->widget()) delete item->widget();
            delete item;
        }

        auto allCards = db.getAllCards();

        std::map<std::string, std::map<std::string, RawTreeBuilderNode>> chapterTrees;
        std::map<std::string, int> chapterCardCounts;

        for (const auto& c : allCards) {
            std::string ch  = c.chapter.empty() ? "Общее" : c.chapter;
            std::string sec = c.section.empty() ? "Заметки" : c.section;
            insertSectionPathIntoTree(chapterTrees[ch], sec, 1);
            chapterCardCounts[ch]++;
        }

        std::error_code ec;
        for (auto& [chName, rootMap] : chapterTrees) {
            fs::path chFolder = db.resolveCardFolder(chName, "all");
            if (fs::exists(chFolder, ec) && fs::is_directory(chFolder, ec)) {
                for (const auto& entry : fs::recursive_directory_iterator(chFolder, fs::directory_options::skip_permission_denied, ec)) {
                    if (!entry.is_directory(ec)) continue;
                    std::string dname = filenameToUtf8(entry.path());
                    if (!dname.empty() && dname[0] == '.') continue;

                    fs::path relToCh = fs::relative(entry.path(), chFolder, ec);
                    std::vector<std::string> parts;
                    for (const auto& p : relToCh) {
                        std::string ps = p.u8string();
                        if (!ps.empty() && ps != "." && ps != "/" && ps != "\\") parts.push_back(ps);
                    }
                    if (!parts.empty() && rootMap.count(parts[0]) > 0) {
                        std::string joined;
                        for (size_t i = 0; i < parts.size(); ++i) {
                            if (i > 0) joined += "/";
                            joined += parts[i];
                        }
                        insertSectionPathIntoTree(rootMap, joined, 0);
                    }
                }
            }
        }

        if (chapterTrees.empty()) {
            overallProgressBar->setValue(0);
            overallStatLabel->setText("Общий прогресс: 0%   •   В базе данных пока нет карточек (0)");
            QLabel* emptyLbl = new QLabel(
                "База данных пуста. Создайте карточку («+ Карточка»), и её глава и дерево директорий автоматически появятся в трекере.",
                                          this
            );
            emptyLbl->setAlignment(Qt::AlignCenter);
            emptyLbl->setWordWrap(true);
            emptyLbl->setContentsMargins(20, 40, 20, 40);
            trackerListLayout->addWidget(emptyLbl);
            trackerListLayout->addStretch(1);
            return;
        }

        struct BuiltChapterTracker {
            std::string chapterName;
            int totalDirs = 0;
            int startedDirs = 0;
            int totalCards = 0;
            std::vector<TrackerDirNode> roots;
        };

        std::vector<BuiltChapterTracker> builtChapters;
        int grandTotalDirs = 0;
        int grandCompletedDirs = 0;

        for (const auto& [courseName, rawRootsMap] : chapterTrees) {
            BuiltChapterTracker bct;
            bct.chapterName = courseName;
            bct.totalCards = chapterCardCounts[courseName];

            for (const auto& [rootKey, rawNode] : rawRootsMap) {
                TrackerDirNode node = finalizeTrackerTree(rawNode, bct.totalDirs, bct.startedDirs, grandCompletedDirs);
                bct.roots.push_back(node);
            }

            std::sort(bct.roots.begin(), bct.roots.end(), [](const TrackerDirNode& a, const TrackerDirNode& b) {
                int ia = std::atoi(a.name.c_str());
                int ib = std::atoi(b.name.c_str());
                if (ia > 0 && ib > 0 && ia != ib) return ia < ib;
                return a.name < b.name;
            });

            grandTotalDirs += bct.totalDirs;
            builtChapters.push_back(std::move(bct));
        }

        for (const auto& bct : builtChapters) {
            int coursePercentPermille = cardsToPermille(bct.totalCards);
            double courseWeightShare = (grandTotalDirs > 0)
            ? (static_cast<double>(bct.totalDirs) / grandTotalDirs) * 100.0
            : 0.0;

            std::string courseName = bct.chapterName;
            TrackerChapterCard* chapterCard = new TrackerChapterCard(
                courseName,
                bct.startedDirs,
                bct.totalDirs,
                bct.totalCards,
                courseWeightShare,
                coursePercentPermille,
                bct.roots,
                [this, courseName](std::string targetSectionPath) {
                    Card c;
                    c.chapter = courseName;
                    c.section = targetSectionPath;
                    CardEditorDialog dlg(c, true, db, this);
                    if (dlg.exec() == QDialog::Accepted) {
                        updateWatchedPaths();
                        refreshAllViews();
                    }
                },
                this
            );

            trackerListLayout->addWidget(chapterCard);
        }

        trackerListLayout->addStretch(1);

        int overallPermille = cardsToPermille(static_cast<int>(allCards.size()));
        overallProgressBar->setValue(overallPermille);
        QString summary = QString("Общий прогресс: %1%   •   Директорий в базе: %2   •   Завершено (100%): %3   •   Всего карточек: %4 из %5 (лимит 100%)")
        .arg(overallPermille / 10.0, 0, 'f', 1)
        .arg(grandTotalDirs)
        .arg(grandCompletedDirs)
        .arg(allCards.size())
        .arg(g_cfg.trackerCardsForFull);
        overallStatLabel->setText(summary);
    }

    void applyModernThemePalette() {
        QFont appFont = QApplication::font();
        appFont.setPointSize(g_cfg.uiFontSize);
        QApplication::setFont(appFont);

        QPalette pal;
        pal.setColor(QPalette::Window, g_cfg.bgWindowTop);
        pal.setColor(QPalette::WindowText, QColor(235, 242, 255));
        pal.setColor(QPalette::Base, g_cfg.bgInputField);
        pal.setColor(QPalette::AlternateBase, g_cfg.bgTrackerCard);
        pal.setColor(QPalette::Text, QColor(248, 250, 252));
        pal.setColor(QPalette::PlaceholderText, QColor(148, 163, 184));
        pal.setColor(QPalette::Button, g_cfg.bgButton);
        pal.setColor(QPalette::ButtonText, g_cfg.isMaleTheme ? QColor(56, 189, 248) : QColor(244, 114, 182));
        pal.setColor(QPalette::Highlight, g_cfg.isMaleTheme ? QColor(14, 165, 233) : QColor(219, 39, 119));
        pal.setColor(QPalette::HighlightedText, Qt::white);
        QApplication::setPalette(pal);
    }
};

int main(int argc, char* argv[]) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    QApplication::setStyle(QStyleFactory::create("Fusion"));

    MainWindow w;
    w.show();
    return app.exec();
}
