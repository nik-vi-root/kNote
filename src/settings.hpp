#pragma once
#include "common.hpp"
#include <QApplication>
#include <QCoreApplication>
#include <QAbstractItemModel>
#include <QAbstractScrollArea>
#include <QKeyEvent>
#include <QLineEdit>
#include <QPainter>
#include <QPlainTextEdit>
#include <QPointer>
#include <QRegularExpression>
#include <QSet>
#include <QSettings>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QTextEdit>
#include <QTimer>
#include <QTreeWidget>
#include <QTreeWidgetItemIterator>
#include <QVector>
#include <algorithm>
#include <cmath>
#include <type_traits>

// ============================================================================
// 1. МОСТ НАСТРОЕК (Шрифт, карточка 400x600, ширина левой и правой панелей,
//    включение/выключение поля «Код / команда» — по умолчанию выключено)
// ============================================================================
namespace CfgBridge {
    inline int  s_uiFontSize             = 13;
    inline int  s_editorFontSize         = 14;
    inline int  s_cardFixedWidth         = 400; // По умолчанию ширина карточки 400 px
    inline int  s_cardFixedHeight        = 600; // По умолчанию высота карточки 600 px
    inline bool s_rememberWindowGeometry = true;
    inline QString s_hkNextField         = "Ctrl+Tab";
    inline QString s_hkInsertCode        = "Ctrl+K";

    template <typename T, typename = void>
    struct HasUiFontSize : std::false_type {};
    template <typename T>
    struct HasUiFontSize<T, std::void_t<decltype(std::declval<T&>().uiFontSize)>> : std::true_type {};

    template <typename T, typename = void>
    struct HasEditorFontSize : std::false_type {};
    template <typename T>
    struct HasEditorFontSize<T, std::void_t<decltype(std::declval<T&>().editorFontSize)>> : std::true_type {};

    template <typename T, typename = void>
    struct HasHkNextField : std::false_type {};
    template <typename T>
    struct HasHkNextField<T, std::void_t<decltype(std::declval<T&>().hkNextField)>> : std::true_type {};

    template <typename T>
    int& getUiFontSize(T& cfg) {
        if constexpr (HasUiFontSize<T>::value) return cfg.uiFontSize;
        else return s_uiFontSize;
    }

    template <typename T>
    int& getEditorFontSize(T& cfg) {
        if constexpr (HasEditorFontSize<T>::value) return cfg.editorFontSize;
        else return s_editorFontSize;
    }

    template <typename T>
    QString& getHkNextField(T& cfg) {
        if constexpr (HasHkNextField<T>::value) return cfg.hkNextField;
        else return s_hkNextField;
    }

    inline int& getCardFixedWidth()          { return s_cardFixedWidth; }
    inline int& getCardFixedHeight()         { return s_cardFixedHeight; }
    inline bool& getRememberWindowGeometry() { return s_rememberWindowGeometry; }
    inline QString& getHkInsertCode()        { return s_hkInsertCode; }

    inline void syncScaleFromWidth() {
        g_cfg.cardScalePercent = std::clamp(static_cast<int>(std::round(s_cardFixedWidth * 100.0 / 415.0)), 60, 180);
    }

    inline void loadExtraSettings() {
        QSettings qs("KnowledgeVault", "AppConfigExtra");
        int uiPt       = qs.value("uiFontSize", 13).toInt();
        int edPt       = qs.value("editorFontSize", 14).toInt();
        int cardW      = qs.value("cardFixedWidthExact", 400).toInt();
        int cardH      = qs.value("cardFixedHeightExact", 600).toInt();
        int leftW      = qs.value("leftPanelWidthExact", g_cfg.leftPanelWidth).toInt();
        int drW        = qs.value("rightDrawerWidthExact", g_cfg.rightDrawerWidth).toInt();
        bool topBar    = qs.value("showTopSectionBar", g_cfg.showTopSectionBar).toBool();
        bool codeField = qs.value("showCodeField", false).toBool();
        int trLim      = qs.value("trackerCardsForFull", g_cfg.trackerCardsForFull).toInt();
        bool remGeo    = qs.value("rememberWindowGeometry", true).toBool();
        QString hkField = qs.value("hkNextField", "Ctrl+Tab").toString();
        QString hkCode  = qs.value("hkInsertCode", "Ctrl+K").toString();

        getUiFontSize(g_cfg)      = std::clamp(uiPt, 8, 24);
        getEditorFontSize(g_cfg)  = std::clamp(edPt, 9, 28);
        s_cardFixedWidth          = std::clamp(cardW, 260, 800);
        s_cardFixedHeight         = std::clamp(cardH, 240, 950);
        g_cfg.leftPanelWidth      = std::clamp(leftW, 200, 800);
        g_cfg.rightDrawerWidth    = std::clamp(drW, 240, 950);
        g_cfg.showTopSectionBar   = topBar;
        g_cfg.showCodeField       = codeField;
        g_cfg.trackerCardsForFull = std::clamp(trLim, 1, 100000);
        s_rememberWindowGeometry  = remGeo;
        getHkNextField(g_cfg)     = hkField;
        s_hkInsertCode            = hkCode;
        syncScaleFromWidth();
    }

    inline void saveExtraSettings() {
        syncScaleFromWidth();
        QSettings qs("KnowledgeVault", "AppConfigExtra");
        qs.setValue("uiFontSize", getUiFontSize(g_cfg));
        qs.setValue("editorFontSize", getEditorFontSize(g_cfg));
        qs.setValue("cardFixedWidthExact", s_cardFixedWidth);
        qs.setValue("cardFixedHeightExact", s_cardFixedHeight);
        qs.setValue("leftPanelWidthExact", g_cfg.leftPanelWidth);
        qs.setValue("rightDrawerWidthExact", g_cfg.rightDrawerWidth);
        qs.setValue("showTopSectionBar", g_cfg.showTopSectionBar);
        qs.setValue("showCodeField", g_cfg.showCodeField);
        qs.setValue("trackerCardsForFull", g_cfg.trackerCardsForFull);
        qs.setValue("rememberWindowGeometry", s_rememberWindowGeometry);
        qs.setValue("hkNextField", getHkNextField(g_cfg));
        qs.setValue("hkInsertCode", s_hkInsertCode);
    }

    inline void saveWindowRect(const QString& key, QWidget* w) {
        if (!s_rememberWindowGeometry || !w) return;
        QRect r = w->geometry();
        if (r.width() < 120 || r.height() < 120) return;
        QSettings qs("KnowledgeVault", "AppConfigExtra");
        qs.setValue(key, r);
    }

    inline bool restoreWindowRect(const QString& key, QWidget* w, const QSize& fallbackSize) {
        if (!w) return false;
        if (!s_rememberWindowGeometry) {
            w->resize(fallbackSize);
            return false;
        }
        QSettings qs("KnowledgeVault", "AppConfigExtra");
        if (!qs.contains(key)) {
            w->resize(fallbackSize);
            return false;
        }
        QRect r = qs.value(key).toRect();
        if (!r.isValid() || r.width() < 120 || r.height() < 120) {
            w->resize(fallbackSize);
            return false;
        }

        QScreen* scr = QGuiApplication::screenAt(r.center());
        if (!scr) scr = QGuiApplication::primaryScreen();
        if (scr) {
            QRect avail = scr->availableGeometry();
            int wClamped = std::min(r.width(), avail.width());
            int hClamped = std::min(r.height(), avail.height());
            int xClamped = std::clamp(r.x(), avail.left(), std::max(avail.left(), avail.right() - wClamped));
            int yClamped = std::clamp(r.y(), avail.top(),  std::max(avail.top(),  avail.bottom() - hClamped));
            r = QRect(xClamped, yClamped, wClamped, hClamped);
        }

        w->setGeometry(r);
        return true;
    }

    inline void applyCardSizeToWidget(QWidget* card) {
        if (!card) return;
        int w = std::clamp(s_cardFixedWidth, 260, 800);
        int h = std::clamp(s_cardFixedHeight, 240, 950);

        if (card->width() != w || card->height() != h) {
            card->setFixedSize(w, h);
        }

        int innerW = std::max(160, w - 40);
        if (QLineEdit* titleEdit = card->findChild<QLineEdit*>()) {
            if (QWidget* titleFrame = titleEdit->parentWidget()) {
                titleFrame->setMaximumWidth(std::max(120, innerW - 68));
            }
        }
        const auto labels = card->findChildren<QLabel*>();
        for (QLabel* lbl : labels) {
            lbl->setMaximumWidth(innerW);
        }
        const auto buttons = card->findChildren<QPushButton*>();
        for (QPushButton* b : buttons) {
            if (b->text().contains("Подроб")) {
                b->setMaximumWidth(innerW);
            }
        }

        QPlainTextEdit* codeEdit = card->findChild<QPlainTextEdit*>();
        QTextEdit* descEdit      = card->findChild<QTextEdit*>();
        if (!descEdit) return;

        QWidget* codeBox = (codeEdit && codeEdit->parentWidget() != card) ? codeEdit->parentWidget() : nullptr;
        QWidget* descBox = (descEdit && descEdit->parentWidget() != card) ? descEdit->parentWidget() : nullptr;
        if (!descBox) return;

        descBox->setMinimumHeight(42);
        descBox->setMaximumHeight(QWIDGETSIZE_MAX);
        descBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);

        // Если поле «Код / команда» выключено (по умолчанию), то поле описания карточки всегда открыто на всю высоту!
        if (!g_cfg.showCodeField) {
            if (codeBox) codeBox->setVisible(false);
            descBox->setVisible(true);
        } else if (codeBox) {
            codeBox->setMinimumHeight(42);
            codeBox->setMaximumHeight(QWIDGETSIZE_MAX);
            codeBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Ignored);
        }

        if (QVBoxLayout* vLayout = qobject_cast<QVBoxLayout*>(card->layout())) {
            if (codeBox) {
                int codeIdx = vLayout->indexOf(codeBox);
                if (codeIdx != -1) vLayout->setStretch(codeIdx, g_cfg.showCodeField ? 1 : 0);
            }
            int descIdx = vLayout->indexOf(descBox);
            if (descIdx != -1) vLayout->setStretch(descIdx, descBox->isVisible() ? 1 : 0);
        }
    }

    inline void applyFontToWidget(QWidget* w) {
        if (!w) return;
        int uiPt = getUiFontSize(g_cfg);
        int edPt = getEditorFontSize(g_cfg);

        QFont wf = w->font();
        if (w->inherits("QTextEdit") || w->inherits("QPlainTextEdit") || w->inherits("QLineEdit")) {
            wf.setPointSize(edPt);
        } else {
            wf.setPointSize(uiPt);
        }
        w->setFont(wf);

        if (w->property("_isCardBlock").toBool()) {
            applyCardSizeToWidget(w);
        }
    }

    inline void applyGlobalFontSizes() {
        int uiPt = getUiFontSize(g_cfg);
        QFont appFont = QApplication::font();
        appFont.setPointSize(uiPt);
        QApplication::setFont(appFont);

        const auto topWidgets = QApplication::topLevelWidgets();
        for (QWidget* top : topWidgets) {
            if (!top) continue;
            applyFontToWidget(top);
            const auto children = top->findChildren<QWidget*>();
            for (QWidget* w : children) {
                applyFontToWidget(w);
            }
        }
    }
}

// ============================================================================
// 2. АВТОМАТИЧЕСКИЙ КОНТРОЛЛЕР КАРТОЧЕК И ДЕРЕВА ПАПОК
//    Когда поле кода выключено (по умолчанию), поле описания открыто сразу!
// ============================================================================
class VaultAutoEnhancer : public QObject {
    QSet<QString> m_expandedTreeKeys;
    QString m_selectedTreeKey;
    bool m_treeRebuilding = false;

    static QString cleanFolderTitle(QString text) {
        static QRegularExpression re("\\s*[\\(\\[\\{]\\d+[\\)\\]\\}]\\s*$");
        return text.replace(re, "").trimmed();
    }

    static QString getTreeItemKey(QTreeWidgetItem* item) {
        if (!item) return QString();
        QStringList parts;
        QTreeWidgetItem* cur = item;
        while (cur) {
            QString roleId = cur->data(0, Qt::UserRole).toString();
            QString name = cleanFolderTitle(cur->text(0));
            parts.prepend(roleId.isEmpty() ? name : (roleId + ":" + name));
            cur = cur->parent();
        }
        return parts.join("/");
    }

    static QWidget* findCardContainer(QWidget* startWidget) {
        QWidget* cur = startWidget;
        while (cur && cur->parentWidget()) {
            if (cur->isWindow()) break;
            const auto btns = cur->findChildren<QPushButton*>();
            bool hasCardActionBtn = false;
            for (QPushButton* b : btns) {
                QString t = b->text() + " " + b->toolTip();
                if (t.contains("📝") || t.contains("Подроб") || t.contains("🗑") || t.contains("★") || t.contains("☆")) {
                    hasCardActionBtn = true;
                    break;
                }
            }
            if (hasCardActionBtn && cur->parentWidget() && !cur->parentWidget()->isWindow()) {
                return cur;
            }
            cur = cur->parentWidget();
        }
        return nullptr;
    }

    static QVector<QWidget*> getOrderedCardFields(QWidget* card) {
        QVector<QWidget*> fields;
        if (!card) return fields;

        const auto children = card->findChildren<QWidget*>();
        for (QWidget* w : children) {
            if (!w->isVisible() || !w->isEnabled()) continue;
            if (auto* le = qobject_cast<QLineEdit*>(w)) {
                if (!le->isReadOnly()) fields.append(w);
            } else if (auto* te = qobject_cast<QTextEdit*>(w)) {
                if (!te->isReadOnly()) fields.append(w);
            } else if (auto* pte = qobject_cast<QPlainTextEdit*>(w)) {
                if (!pte->isReadOnly()) fields.append(w);
            }
        }

        std::sort(fields.begin(), fields.end(), [](QWidget* a, QWidget* b) {
            QPoint pa = a->mapToGlobal(QPoint(0, 0));
            QPoint pb = b->mapToGlobal(QPoint(0, 0));
            if (std::abs(pa.y() - pb.y()) > 10) return pa.y() < pb.y();
            return pa.x() < pb.x();
        });
        return fields;
    }

    static void triggerCardSave(QWidget* card, QPushButton* saveBtn = nullptr) {
        if (!card) return;

        const auto lines = card->findChildren<QLineEdit*>();
        for (QLineEdit* le : lines) {
            QMetaObject::invokeMethod(le, "editingFinished", Qt::DirectConnection);
            QMetaObject::invokeMethod(le, "returnPressed", Qt::DirectConnection);
        }

        const auto texts = card->findChildren<QTextEdit*>();
        for (QTextEdit* te : texts) {
            QMetaObject::invokeMethod(te, "textChanged", Qt::DirectConnection);
        }

        const auto plainTexts = card->findChildren<QPlainTextEdit*>();
        for (QPlainTextEdit* pte : plainTexts) {
            QMetaObject::invokeMethod(pte, "textChanged", Qt::DirectConnection);
        }

        QWidget* prevFocus = QApplication::focusWidget();
        if (prevFocus && card->isAncestorOf(prevFocus)) {
            prevFocus->clearFocus();
            prevFocus->setFocus(Qt::OtherFocusReason);
        }

        if (saveBtn) {
            saveBtn->setText("✔");
            QPointer<QPushButton> safeBtn(saveBtn);
            QTimer::singleShot(900, [safeBtn]() {
                if (safeBtn) safeBtn->setText("💾");
            });
        }
    }

    static bool insertCodeFenceIntoFocusedEditor() {
        QWidget* fw = QApplication::focusWidget();
        if (!fw) return false;

        if (QWidget* p = fw->parentWidget()) {
            if (qobject_cast<QTextEdit*>(p) || qobject_cast<QPlainTextEdit*>(p)) {
                fw = p;
            }
        }

        if (QTextEdit* te = qobject_cast<QTextEdit*>(fw)) {
            if (te->isReadOnly()) return false;
            QTextCursor cursor = te->textCursor();
            QTextCharFormat cleanCodeFmt;
            cleanCodeFmt.setFontFamily(MONO_FONT_FAMILY);
            cleanCodeFmt.setFontPointSize(CfgBridge::getEditorFontSize(g_cfg));
            cleanCodeFmt.setFontWeight(QFont::Normal);
            cleanCodeFmt.setFontUnderline(false);
            cleanCodeFmt.setForeground(QColor(240, 246, 255));
            cleanCodeFmt.clearBackground();

            cursor.beginEditBlock();
            if (cursor.hasSelection()) {
                QString sel = cursor.selectedText();
                sel.replace(QChar::ParagraphSeparator, '\n');
                sel.replace(QChar::LineSeparator, '\n');
                cursor.insertText("```\n" + sel + "\n```", cleanCodeFmt);
            } else {
                if (!cursor.atBlockStart()) cursor.insertText("\n", cleanCodeFmt);
                cursor.insertText("```\n\n```", cleanCodeFmt);
                cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 4);
                te->setTextCursor(cursor);
            }
            cursor.endEditBlock();
            return true;
        }

        if (QPlainTextEdit* pte = qobject_cast<QPlainTextEdit*>(fw)) {
            if (pte->isReadOnly()) return false;
            QTextCursor cursor = pte->textCursor();
            cursor.beginEditBlock();
            if (cursor.hasSelection()) {
                QString sel = cursor.selectedText();
                sel.replace(QChar::ParagraphSeparator, '\n');
                sel.replace(QChar::LineSeparator, '\n');
                cursor.insertText("```\n" + sel + "\n```");
            } else {
                if (!cursor.atBlockStart()) cursor.insertText("\n");
                cursor.insertText("```\n\n```");
                cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 4);
                pte->setTextCursor(cursor);
            }
            cursor.endEditBlock();
            return true;
        }

        return false;
    }

    void setupCardEnhancements(QPushButton* detailsBtn) {
        if (!detailsBtn || !detailsBtn->parentWidget()) return;
        QWidget* parentW = detailsBtn->parentWidget();

        QWidget* card = findCardContainer(detailsBtn);
        if (!card) card = parentW;

        card->setProperty("_isCardBlock", true);

        if (card->property("_hasSaveBtnInjected").toBool()) {
            CfgBridge::applyCardSizeToWidget(card);
            return;
        }

        QHBoxLayout* targetLayout = qobject_cast<QHBoxLayout*>(parentW->layout());
        if (!targetLayout && card->layout()) {
            const auto layouts = card->findChildren<QHBoxLayout*>();
            for (QHBoxLayout* l : layouts) {
                if (l->indexOf(detailsBtn) != -1) {
                    targetLayout = l;
                    break;
                }
            }
        }
        if (!targetLayout) return;

        card->setProperty("_hasSaveBtnInjected", true);

        QTextEdit* descEdit = card->findChild<QTextEdit*>();
        QWidget* descBox = (descEdit && descEdit->parentWidget() != card) ? descEdit->parentWidget() : nullptr;

        int btnH = std::max(28, detailsBtn->height());

        NeonButton* saveBtn = new NeonButton("💾", QColor("#4ade80"), parentW);
        saveBtn->setToolTip("Сохранить изменения в карточке (Ctrl+S)");
        saveBtn->setFixedSize(36, btnH);
        saveBtn->setCursor(Qt::PointingHandCursor);

        int idx = targetLayout->indexOf(detailsBtn);
        if (idx >= 0) {
            targetLayout->insertWidget(idx, saveBtn);
        } else {
            targetLayout->addWidget(saveBtn);
        }

        QPointer<QWidget> safeCard(card);
        QPointer<QPushButton> safeSaveBtn(saveBtn);
        connect(saveBtn, &QPushButton::clicked, [safeCard, safeSaveBtn]() {
            triggerCardSave(safeCard, safeSaveBtn);
        });

        // Только если включено дополнительное поле «Код / команда», добавляем кнопку сворачивания описания +📓.
        // По умолчанию (g_cfg.showCodeField == false) поле описания всегда открыто!
        if (g_cfg.showCodeField && descBox) {
            descBox->setVisible(false);
            bool hasDescText = (descEdit && !descEdit->toPlainText().trimmed().isEmpty());
            NeonButton* toggleDescBtn = new NeonButton(hasDescText ? "+📓 ●" : "+📓", QColor("#c084fc"), parentW);
            toggleDescBtn->setToolTip("Открыть / скрыть поле описания в карточке");
            toggleDescBtn->setFixedSize(hasDescText ? 58 : 48, btnH);
            toggleDescBtn->setCursor(Qt::PointingHandCursor);

            int dIdx = targetLayout->indexOf(detailsBtn);
            if (dIdx >= 0) targetLayout->insertWidget(dIdx, toggleDescBtn);
            else targetLayout->addWidget(toggleDescBtn);

            QPointer<QWidget> safeDescBox(descBox);
            QPointer<QTextEdit> safeDescEdit(descEdit);
            QPointer<NeonButton> safeToggleBtn(toggleDescBtn);

            connect(toggleDescBtn, &QPushButton::clicked, [safeCard, safeDescBox, safeDescEdit, safeToggleBtn]() {
                if (!safeCard || !safeDescBox || !safeToggleBtn) return;
                bool willShow = !safeDescBox->isVisible();
                safeDescBox->setVisible(willShow);
                CfgBridge::applyCardSizeToWidget(safeCard);

                bool hasTxt = (safeDescEdit && !safeDescEdit->toPlainText().trimmed().isEmpty());
                if (willShow) {
                    safeToggleBtn->setText("📓−");
                    safeToggleBtn->setFixedWidth(48);
                    if (safeDescEdit) safeDescEdit->setFocus();
                } else {
                    safeToggleBtn->setText(hasTxt ? "+📓 ●" : "+📓");
                    safeToggleBtn->setFixedWidth(hasTxt ? 58 : 48);
                }
            });
        } else if (descBox) {
            descBox->setVisible(true);
        }

        CfgBridge::applyCardSizeToWidget(card);
    }

    void hookTreeWidget(QTreeWidget* tree) {
        if (!tree || tree->property("_vaultTreeHooked").toBool()) return;
        tree->setProperty("_vaultTreeHooked", true);

        QTreeWidgetItemIterator it(tree);
        while (*it) {
            if ((*it)->isExpanded()) {
                m_expandedTreeKeys.insert(getTreeItemKey(*it));
            }
            ++it;
        }

        connect(tree, &QTreeWidget::itemExpanded, this, [this](QTreeWidgetItem* item) {
            if (item) m_expandedTreeKeys.insert(getTreeItemKey(item));
        });

            connect(tree, &QTreeWidget::itemCollapsed, this, [this](QTreeWidgetItem* item) {
                if (!m_treeRebuilding && item) {
                    m_expandedTreeKeys.remove(getTreeItemKey(item));
                }
            });

            connect(tree, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* cur, QTreeWidgetItem*) {
                if (!m_treeRebuilding && cur) {
                    m_selectedTreeKey = getTreeItemKey(cur);
                }
            });

            if (QAbstractItemModel* model = tree->model()) {
                QPointer<QTreeWidget> safeTree(tree);

                auto snapshotBeforeClear = [this, safeTree]() {
                    if (!safeTree) return;
                    m_treeRebuilding = true;
                    if (safeTree->currentItem()) {
                        m_selectedTreeKey = getTreeItemKey(safeTree->currentItem());
                    }
                    QTreeWidgetItemIterator it(safeTree);
                    while (*it) {
                        if ((*it)->isExpanded()) {
                            m_expandedTreeKeys.insert(getTreeItemKey(*it));
                        }
                        ++it;
                    }
                };

                auto restoreAfterPopulate = [this, safeTree]() {
                    QTimer::singleShot(0, this, [this, safeTree]() {
                        if (!safeTree) return;
                        QSignalBlocker blocker(safeTree);
                        QTreeWidgetItemIterator it(safeTree);
                        while (*it) {
                            QString key = getTreeItemKey(*it);
                            if (m_expandedTreeKeys.contains(key)) {
                                (*it)->setExpanded(true);
                            }
                            if (!m_selectedTreeKey.isEmpty() && key == m_selectedTreeKey) {
                                safeTree->setCurrentItem(*it);
                                QTreeWidgetItem* p = (*it)->parent();
                                while (p) {
                                    p->setExpanded(true);
                                    m_expandedTreeKeys.insert(getTreeItemKey(p));
                                    p = p->parent();
                                }
                            }
                            ++it;
                        }
                        m_treeRebuilding = false;
                    });
                };

                connect(model, &QAbstractItemModel::modelAboutToBeReset, this, snapshotBeforeClear);
                connect(model, &QAbstractItemModel::rowsAboutToBeRemoved, this, snapshotBeforeClear);
                connect(model, &QAbstractItemModel::modelReset, this, restoreAfterPopulate);
                connect(model, &QAbstractItemModel::rowsInserted, this, restoreAfterPopulate);
            }
    }

public:
    explicit VaultAutoEnhancer(QObject* parent = nullptr) : QObject(parent) {}

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::Show) {
            if (QWidget* w = qobject_cast<QWidget*>(watched)) {
                CfgBridge::applyFontToWidget(w);

                if (QTreeWidget* tree = qobject_cast<QTreeWidget*>(w)) {
                    hookTreeWidget(tree);
                } else if (QPushButton* btn = qobject_cast<QPushButton*>(w)) {
                    QString t = btn->text() + " " + btn->toolTip();
                    if (t.contains("📝") || t.contains("Подроб")) {
                        setupCardEnhancements(btn);
                    }
                }
            }
        }
        else if (event->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            QKeySequence pressedSeq(ke->modifiers() | ke->key());

            if ((ke->modifiers() & Qt::ControlModifier) && ke->key() == Qt::Key_S) {
                if (QWidget* fw = QApplication::focusWidget()) {
                    if (QWidget* card = findCardContainer(fw)) {
                        QPushButton* saveBtn = nullptr;
                        const auto btns = card->findChildren<QPushButton*>();
                        for (QPushButton* b : btns) {
                            if (b->text() == "💾" || b->text() == "✔") {
                                saveBtn = b;
                                break;
                            }
                        }
                        triggerCardSave(card, saveBtn);
                        return true;
                    }
                }
            }

            QKeySequence codeSeq(CfgBridge::getHkInsertCode());
            if (!codeSeq.isEmpty() && pressedSeq.matches(codeSeq) == QKeySequence::ExactMatch) {
                if (insertCodeFenceIntoFocusedEditor()) {
                    return true;
                }
            }

            QKeySequence targetSeq(CfgBridge::getHkNextField(g_cfg));
            bool isCtrlTab = ((ke->modifiers() & Qt::ControlModifier) &&
            (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab));
            bool isCustomHk = (!targetSeq.isEmpty() && pressedSeq.matches(targetSeq) == QKeySequence::ExactMatch);

            if (isCtrlTab || isCustomHk) {
                QWidget* fw = QApplication::focusWidget();
                if (!fw) return false;

                if (QWidget* p = fw->parentWidget()) {
                    if (qobject_cast<QTextEdit*>(p) || qobject_cast<QPlainTextEdit*>(p)) {
                        fw = p;
                    }
                }

                if (QWidget* card = findCardContainer(fw)) {
                    QVector<QWidget*> fields = getOrderedCardFields(card);
                    int idx = fields.indexOf(fw);
                    if (idx != -1) {
                        if (idx + 1 < fields.size()) {
                            QWidget* nextField = fields[idx + 1];
                            nextField->setFocus(Qt::ShortcutFocusReason);
                            if (QLineEdit* le = qobject_cast<QLineEdit*>(nextField)) {
                                le->selectAll();
                            }
                        } else {
                            QPushButton* detailsBtn = nullptr;
                            const auto btns = card->findChildren<QPushButton*>();
                            for (QPushButton* b : btns) {
                                QString t = b->text() + " " + b->toolTip();
                                if (t.contains("📝") || t.contains("Подроб")) {
                                    detailsBtn = b;
                                    break;
                                }
                            }
                            if (detailsBtn) {
                                detailsBtn->click();
                            }
                            QPointer<QWidget> safeCard(card);
                            QTimer::singleShot(50, this, [safeCard]() {
                                QWidget* activeWin = QApplication::activeWindow();
                                if (!activeWin) return;

                                const auto textEdits = activeWin->findChildren<QTextEdit*>();
                                for (QTextEdit* te : textEdits) {
                                    if (te->isVisible() && (!safeCard || !safeCard->isAncestorOf(te))) {
                                        te->setFocus(Qt::ShortcutFocusReason);
                                        return;
                                    }
                                }
                                const auto plainEdits = activeWin->findChildren<QPlainTextEdit*>();
                                for (QPlainTextEdit* pe : plainEdits) {
                                    if (pe->isVisible() && (!safeCard || !safeCard->isAncestorOf(pe))) {
                                        pe->setFocus(Qt::ShortcutFocusReason);
                                        return;
                                    }
                                }
                            });
                        }
                        return true;
                    }
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

inline void installVaultAutoEnhancer() {
    if (!qApp) return;
    CfgBridge::loadExtraSettings();
    auto* enhancer = new VaultAutoEnhancer(qApp);
    qApp->installEventFilter(enhancer);

    QTimer::singleShot(80, []() {
        CfgBridge::loadExtraSettings();
        CfgBridge::applyGlobalFontSizes();
    });
}
Q_COREAPP_STARTUP_FUNCTION(installVaultAutoEnhancer)

// ============================================================================
// 3. ОКНО НАСТРОЕК (Включая настройку отдельного поля «Код / команда»)
// ============================================================================
class SettingsDialog : public QDialog {
protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QLinearGradient bg(0, 0, 0, height());
        bg.setColorAt(0.0, QColor(15, 23, 42));
        bg.setColorAt(1.0, QColor(10, 15, 30));
        p.fillRect(rect(), bg);

        QPen borderPen(QColor(56, 189, 248, 90), 1.5);
        p.setPen(borderPen);
        p.setBrush(Qt::NoBrush);
        p.drawRoundedRect(rect().adjusted(1, 1, -1, -1), 10, 10);
    }

public:
    SettingsDialog(DatabaseManager& db, std::function<void()> onApplyLive, QWidget* parent = nullptr)
    : QDialog(parent)
    {
        CfgBridge::loadExtraSettings();

        setWindowTitle("⚙ Настройки интерфейса, ширины панелей, полей карточек и трекера");
        resize(900, 770);

        QPalette pal = palette();
        pal.setColor(QPalette::Window, QColor(15, 23, 42));
        pal.setColor(QPalette::WindowText, QColor(226, 232, 240));
        pal.setColor(QPalette::Base, QColor(15, 23, 42));
        pal.setColor(QPalette::Text, QColor(226, 232, 240));
        pal.setColor(QPalette::Button, QColor(30, 41, 59));
        pal.setColor(QPalette::ButtonText, QColor(226, 232, 240));
        setPalette(pal);

        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(20, 18, 20, 18);
        root->setSpacing(14);

        QFont secFont = font();
        secFont.setPointSize(CfgBridge::getUiFontSize(g_cfg) + 1);
        secFont.setBold(true);

        QTabWidget* tabs = new QTabWidget(this);

        // ====================================================================
        // ВКЛАДКА 1: ИНТЕРФЕЙС, ШРИФТ, РАЗМЕРЫ КАРТОЧЕК И ПОЛЕ КОДА
        // ====================================================================
        QWidget* visTab = new QWidget(this);
        QVBoxLayout* visRoot = new QVBoxLayout(visTab);
        visRoot->setContentsMargins(20, 20, 20, 20);
        visRoot->setSpacing(12);

        QLabel* s1 = new QLabel("1. Размер текста, габариты карточек (px) и отображаемые поля", visTab);
        s1->setFont(secFont);
        visRoot->addWidget(s1);

        QGridLayout* visGrid = new QGridLayout();
        visGrid->setHorizontalSpacing(16);
        visGrid->setVerticalSpacing(10);

        int curUiFont     = CfgBridge::getUiFontSize(g_cfg);
        int curEdFont     = CfgBridge::getEditorFontSize(g_cfg);
        int curCardWidth  = std::clamp(CfgBridge::getCardFixedWidth(), 260, 800);
        int curCardHeight = std::clamp(CfgBridge::getCardFixedHeight(), 240, 950);
        int curOpacityPct = std::clamp(g_cfg.cardOpacity * 100 / 255, 35, 100);

        auto makeSpin = [](QWidget* parent, int minV, int maxV, int val, const QString& suffix) {
            QSpinBox* sb = new QSpinBox(parent);
            sb->setRange(minV, maxV);
            sb->setSuffix(suffix);
            sb->setMinimumHeight(32);
            sb->setMinimumWidth(115);
            sb->setValue(val);
            return sb;
        };

        QSlider* uiFontSlider = new QSlider(Qt::Horizontal, visTab);
        uiFontSlider->setRange(9, 22);
        uiFontSlider->setValue(curUiFont);
        QSpinBox* uiFontSpin = makeSpin(visTab, 9, 22, curUiFont, " pt");

        QSlider* edFontSlider = new QSlider(Qt::Horizontal, visTab);
        edFontSlider->setRange(10, 26);
        edFontSlider->setValue(curEdFont);
        QSpinBox* edFontSpin = makeSpin(visTab, 10, 26, curEdFont, " pt");

        QSlider* cardWidthSlider1 = new QSlider(Qt::Horizontal, visTab);
        cardWidthSlider1->setRange(260, 800);
        cardWidthSlider1->setValue(curCardWidth);
        QSpinBox* cardWidthSpin1 = makeSpin(visTab, 260, 800, curCardWidth, " px");

        QSlider* cardHeightSlider1 = new QSlider(Qt::Horizontal, visTab);
        cardHeightSlider1->setRange(240, 950);
        cardHeightSlider1->setValue(curCardHeight);
        QSpinBox* cardHeightSpin1 = makeSpin(visTab, 240, 950, curCardHeight, " px");

        QSlider* opSlider = new QSlider(Qt::Horizontal, visTab);
        opSlider->setRange(35, 100);
        opSlider->setValue(curOpacityPct);
        QSpinBox* opSpin = makeSpin(visTab, 35, 100, curOpacityPct, " %");

        QSlider* blurSlider = new QSlider(Qt::Horizontal, visTab);
        blurSlider->setRange(8, 48);
        blurSlider->setValue(g_cfg.shadowBlurRadius);
        QSpinBox* blurSpin = makeSpin(visTab, 8, 48, g_cfg.shadowBlurRadius, " px");

        visGrid->addWidget(new QLabel("Размер шрифта интерфейса и папок:", visTab), 0, 0);
        visGrid->addWidget(uiFontSlider, 0, 1);
        visGrid->addWidget(uiFontSpin, 0, 2);

        visGrid->addWidget(new QLabel("Размер текста внутри карточек и редактора:", visTab), 1, 0);
        visGrid->addWidget(edFontSlider, 1, 1);
        visGrid->addWidget(edFontSpin, 1, 2);

        visGrid->addWidget(new QLabel("Фиксированная ширина карточек (по умолч. 400 px):", visTab), 2, 0);
        visGrid->addWidget(cardWidthSlider1, 2, 1);
        visGrid->addWidget(cardWidthSpin1, 2, 2);

        visGrid->addWidget(new QLabel("Фиксированная высота карточек (по умолч. 600 px):", visTab), 3, 0);
        visGrid->addWidget(cardHeightSlider1, 3, 1);
        visGrid->addWidget(cardHeightSpin1, 3, 2);

        visGrid->addWidget(new QLabel("Непрозрачность фона карточек:", visTab), 4, 0);
        visGrid->addWidget(opSlider, 4, 1);
        visGrid->addWidget(opSpin, 4, 2);

        visGrid->addWidget(new QLabel("Радиус размытия неоновой тени:", visTab), 5, 0);
        visGrid->addWidget(blurSlider, 5, 1);
        visGrid->addWidget(blurSpin, 5, 2);

        visRoot->addLayout(visGrid);

        QCheckBox* showCodeFieldCheck1 = new QCheckBox("Включить отдельное поле «Код / команда» в карточках (по умолч. выключено — используются блоки ``` ``` в описании)", visTab);
        QCheckBox* showTopBarCheck     = new QCheckBox("Показывать панель отображаемых разделов над поиском (также переключается в меню «☰ Меню»)", visTab);
        QCheckBox* rememberGeomCheck1  = new QCheckBox("Запомнить положение и размер окна?", visTab);
        QCheckBox* glassCheck          = new QCheckBox("Включить стеклянный блик и мягкое внутреннее размытие карточек", visTab);
        QCheckBox* shadowCheck         = new QCheckBox("Включить глубокие неоновые тени вокруг карточек", visTab);
        QCheckBox* hoverCheck          = new QCheckBox("Включить плавную анимацию парения при наведении мыши", visTab);

        showCodeFieldCheck1->setChecked(g_cfg.showCodeField);
        showTopBarCheck->setChecked(g_cfg.showTopSectionBar);
        rememberGeomCheck1->setChecked(CfgBridge::getRememberWindowGeometry());
        glassCheck->setChecked(g_cfg.enableBlurGlass);
        shadowCheck->setChecked(g_cfg.enableShadows);
        hoverCheck->setChecked(g_cfg.enableHoverFloat);

        visRoot->addWidget(showCodeFieldCheck1);
        visRoot->addWidget(showTopBarCheck);
        visRoot->addWidget(rememberGeomCheck1);
        visRoot->addWidget(glassCheck);
        visRoot->addWidget(shadowCheck);
        visRoot->addWidget(hoverCheck);
        visRoot->addStretch(1);

        // ====================================================================
        // ВКЛАДКА 2: ЛЕВАЯ И ПРАВАЯ ПАНЕЛИ (ЧИСЛОВАЯ РЕГУЛИРОВКА ШИРИНЫ)
        // ====================================================================
        QWidget* editorTab = new QWidget(this);
        QVBoxLayout* editorRoot = new QVBoxLayout(editorTab);
        editorRoot->setContentsMargins(20, 20, 20, 20);
        editorRoot->setSpacing(14);

        QLabel* s2 = new QLabel("2. Числовая регулировка ширины левой и правой панелей (px) и поля карточек", editorTab);
        s2->setFont(secFont);
        editorRoot->addWidget(s2);

        QHBoxLayout* modeRow = new QHBoxLayout();
        modeRow->setSpacing(14);
        modeRow->addWidget(new QLabel("Открывать подробное описание (📝) как:", editorTab));
        QComboBox* editorModeCombo = new QComboBox(editorTab);
        editorModeCombo->setMinimumHeight(34);
        editorModeCombo->addItem("Боковая правая панель (выдвигается внутрь окна)", true);
        editorModeCombo->addItem("Отдельное немодальное окно (одновременная работа с карточкой)", false);
        editorModeCombo->setCurrentIndex(g_cfg.useRightDrawer ? 0 : 1);
        modeRow->addWidget(editorModeCombo, 1);
        editorRoot->addLayout(modeRow);

        QGridLayout* edSizeGrid = new QGridLayout();
        edSizeGrid->setHorizontalSpacing(14);
        edSizeGrid->setVerticalSpacing(12);

        int curLeftWidth   = std::clamp(g_cfg.leftPanelWidth, 200, 800);
        int curDrawerWidth = std::clamp(g_cfg.rightDrawerWidth, 240, 950);

        QSlider* leftWidthSlider = new QSlider(Qt::Horizontal, editorTab);
        leftWidthSlider->setRange(200, 800);
        leftWidthSlider->setValue(curLeftWidth);
        QSpinBox* leftWidthSpin = makeSpin(editorTab, 200, 800, curLeftWidth, " px");

        QSlider* drawerWidthSlider = new QSlider(Qt::Horizontal, editorTab);
        drawerWidthSlider->setRange(240, 950);
        drawerWidthSlider->setValue(curDrawerWidth);
        QSpinBox* drawerWidthSpin = makeSpin(editorTab, 240, 950, curDrawerWidth, " px");

        QSlider* cardWidthSlider2 = new QSlider(Qt::Horizontal, editorTab);
        cardWidthSlider2->setRange(260, 800);
        cardWidthSlider2->setValue(curCardWidth);
        QSpinBox* cardWidthSpin2 = makeSpin(editorTab, 260, 800, curCardWidth, " px");

        QSlider* cardHeightSlider2 = new QSlider(Qt::Horizontal, editorTab);
        cardHeightSlider2->setRange(240, 950);
        cardHeightSlider2->setValue(curCardHeight);
        QSpinBox* cardHeightSpin2 = makeSpin(editorTab, 240, 950, curCardHeight, " px");

        edSizeGrid->addWidget(new QLabel("Ширина левой панели проводника (цифрами, по умолч. 315 px):", editorTab), 0, 0);
        edSizeGrid->addWidget(leftWidthSlider, 0, 1);
        edSizeGrid->addWidget(leftWidthSpin, 0, 2);

        edSizeGrid->addWidget(new QLabel("Ширина правой панели описания (цифрами, по умолч. 480 px):", editorTab), 1, 0);
        edSizeGrid->addWidget(drawerWidthSlider, 1, 1);
        edSizeGrid->addWidget(drawerWidthSpin, 1, 2);

        edSizeGrid->addWidget(new QLabel("Фиксированная ширина карточек:", editorTab), 2, 0);
        edSizeGrid->addWidget(cardWidthSlider2, 2, 1);
        edSizeGrid->addWidget(cardWidthSpin2, 2, 2);

        edSizeGrid->addWidget(new QLabel("Фиксированная высота карточек:", editorTab), 3, 0);
        edSizeGrid->addWidget(cardHeightSlider2, 3, 1);
        edSizeGrid->addWidget(cardHeightSpin2, 3, 2);

        editorRoot->addLayout(edSizeGrid);

        QCheckBox* showCodeFieldCheck2 = new QCheckBox("Включить отдельное поле «Код / команда» в карточках (по умолчанию выключено)", editorTab);
        QCheckBox* rememberGeomCheck2  = new QCheckBox("Запомнить положение и размер окна?", editorTab);
        showCodeFieldCheck2->setChecked(g_cfg.showCodeField);
        rememberGeomCheck2->setChecked(CfgBridge::getRememberWindowGeometry());
        editorRoot->addWidget(showCodeFieldCheck2);
        editorRoot->addWidget(rememberGeomCheck2);

        QLabel* fixedHint = new QLabel(
            "• По умолчанию в карточках отображается только поле Описания (с поддержкой блоков ``` ```, тегов Obsidian и прикрепления файлов).\n"
            "• Правая панель («📝 Подробнее») полностью отделена от описания карточки и хранится в секции «## Подробнее».\n"
            "• Ширина левой и правой панелей регулируется цифрами в полях выше.",
            editorTab
        );
        fixedHint->setWordWrap(true);
        editorRoot->addWidget(fixedHint);
        editorRoot->addStretch(1);

        // ====================================================================
        // ВКЛАДКА 3: ГОРЯЧИЕ КЛАВИШИ
        // ====================================================================
        QWidget* hkTab = new QWidget(this);
        QVBoxLayout* hkRoot = new QVBoxLayout(hkTab);
        hkRoot->setContentsMargins(20, 20, 20, 20);
        hkRoot->setSpacing(12);

        QLabel* s3 = new QLabel("3. Назначение горячих клавиш (кликните в поле и нажмите комбинацию)", hkTab);
        s3->setFont(secFont);
        hkRoot->addWidget(s3);

        QGridLayout* hkGrid = new QGridLayout();
        hkGrid->setHorizontalSpacing(18);
        hkGrid->setVerticalSpacing(10);

        QKeySequenceEdit* hkInsertCodeEdit  = new QKeySequenceEdit(QKeySequence(CfgBridge::getHkInsertCode()), hkTab);
        QKeySequenceEdit* hkFormatPopupEdit = new QKeySequenceEdit(QKeySequence(g_cfg.hkFormatPopup), hkTab);
        QKeySequenceEdit* hkNextFieldEdit   = new QKeySequenceEdit(QKeySequence(CfgBridge::getHkNextField(g_cfg)), hkTab);
        QKeySequenceEdit* hkNewCardEdit     = new QKeySequenceEdit(QKeySequence(g_cfg.hkNewCard), hkTab);
        QKeySequenceEdit* hkVaultEdit       = new QKeySequenceEdit(QKeySequence(g_cfg.hkToggleVault), hkTab);
        QKeySequenceEdit* hkDrawerEdit      = new QKeySequenceEdit(QKeySequence(g_cfg.hkToggleDrawer), hkTab);
        QKeySequenceEdit* hkTrackerEdit     = new QKeySequenceEdit(QKeySequence(g_cfg.hkTracker), hkTab);
        QKeySequenceEdit* hkSettingsEdit    = new QKeySequenceEdit(QKeySequence(g_cfg.hkSettings), hkTab);
        QKeySequenceEdit* hkSearchEdit      = new QKeySequenceEdit(QKeySequence(g_cfg.hkFocusSearch), hkTab);

        hkGrid->addWidget(new QLabel("Вставить блок кода ``` ``` в описание / редактор:", hkTab), 0, 0);
        hkGrid->addWidget(new NeonInputFrame(hkInsertCodeEdit, QColor("#4ade80"), hkTab), 0, 1);

        hkGrid->addWidget(new QLabel("Вызвать плавающую док-панель у курсора:", hkTab), 1, 0);
        hkGrid->addWidget(new NeonInputFrame(hkFormatPopupEdit, QColor("#a855f7"), hkTab), 1, 1);

        hkGrid->addWidget(new QLabel("Смена поля в карточке / переход в окно редактора:", hkTab), 2, 0);
        hkGrid->addWidget(new NeonInputFrame(hkNextFieldEdit, QColor("#f472b6"), hkTab), 2, 1);

        hkGrid->addWidget(new QLabel("Создать новую карточку:", hkTab), 3, 0);
        hkGrid->addWidget(new NeonInputFrame(hkNewCardEdit, QColor("#38bdf8"), hkTab), 3, 1);

        hkGrid->addWidget(new QLabel("Открыть / скрыть левую панель глав и папок (◀/▶):", hkTab), 4, 0);
        hkGrid->addWidget(new NeonInputFrame(hkVaultEdit, QColor("#38bdf8"), hkTab), 4, 1);

        hkGrid->addWidget(new QLabel("Открыть / закрыть правую панель редактора (📝):", hkTab), 5, 0);
        hkGrid->addWidget(new NeonInputFrame(hkDrawerEdit, QColor("#38bdf8"), hkTab), 5, 1);

        hkGrid->addWidget(new QLabel("Переключить База знаний / Трекер:", hkTab), 6, 0);
        hkGrid->addWidget(new NeonInputFrame(hkTrackerEdit, QColor("#38bdf8"), hkTab), 6, 1);

        hkGrid->addWidget(new QLabel("Открыть окно настроек:", hkTab), 7, 0);
        hkGrid->addWidget(new NeonInputFrame(hkSettingsEdit, QColor("#38bdf8"), hkTab), 7, 1);

        hkGrid->addWidget(new QLabel("Фокус в строку поиска:", hkTab), 8, 0);
        hkGrid->addWidget(new NeonInputFrame(hkSearchEdit, QColor("#38bdf8"), hkTab), 8, 1);

        hkRoot->addLayout(hkGrid);
        hkRoot->addStretch(1);

        // ====================================================================
        // ВКЛАДКА 4: 📊 ТРЕКЕР (МАКСИМУМ ЛИМИТА ШКАЛЫ)
        // ====================================================================
        QWidget* trackerTab = new QWidget(this);
        QVBoxLayout* trRoot = new QVBoxLayout(trackerTab);
        trRoot->setContentsMargins(20, 20, 20, 20);
        trRoot->setSpacing(16);

        QLabel* s4 = new QLabel("4. 📊 Настройка максимума лимита шкалы в Трекере прогресса", trackerTab);
        s4->setFont(secFont);
        trRoot->addWidget(s4);

        QLabel* trDesc = new QLabel(
            "Укажите, сколько карточек необходимо для заполнения шкалы прогресса на 100%.\n"
            "• При значении 100 пунктов: 1 карточка = 1.0% заполнения шкалы.\n"
            "• При значении 200 пунктов: 1 карточка = 0.5% заполнения шкалы.\n"
            "• При значении 50 пунктов:  1 карточка = 2.0% заполнения шкалы.",
            trackerTab
        );
        trDesc->setWordWrap(true);
        trRoot->addWidget(trDesc);

        int curTrackerLimit = std::clamp(g_cfg.trackerCardsForFull, 1, 100000);

        QGridLayout* trGrid = new QGridLayout();
        trGrid->setHorizontalSpacing(16);
        trGrid->setVerticalSpacing(12);

        QSlider* trackerLimitSlider = new QSlider(Qt::Horizontal, trackerTab);
        trackerLimitSlider->setRange(1, 1000);
        trackerLimitSlider->setValue(std::clamp(curTrackerLimit, 1, 1000));

        QSpinBox* trackerLimitSpin = makeSpin(trackerTab, 1, 100000, curTrackerLimit, " карт.");

        trGrid->addWidget(new QLabel("Лимит шкалы на 100% (количество карточек):", trackerTab), 0, 0);
        trGrid->addWidget(trackerLimitSlider, 0, 1);
        trGrid->addWidget(trackerLimitSpin, 0, 2);
        trRoot->addLayout(trGrid);

        QLabel* presetTitle = new QLabel("Быстрый выбор лимита шкалы:", trackerTab);
        QHBoxLayout* presetRow = new QHBoxLayout();
        presetRow->setSpacing(8);
        for (int presetVal : {25, 50, 100, 150, 200, 300, 500, 1000}) {
            NeonButton* pBtn = new NeonButton(QString::number(presetVal), QColor("#38bdf8"), trackerTab);
            pBtn->setFixedHeight(32);
            connect(pBtn, &QPushButton::clicked, [trackerLimitSpin, presetVal]() {
                trackerLimitSpin->setValue(presetVal);
            });
            presetRow->addWidget(pBtn);
        }
        presetRow->addStretch(1);

        trRoot->addWidget(presetTitle);
        trRoot->addLayout(presetRow);

        QLabel* liveScaleHint = new QLabel(trackerTab);
        liveScaleHint->setFont(secFont);
        QPalette hintPal = liveScaleHint->palette();
        hintPal.setColor(QPalette::WindowText, QColor("#4ade80"));
        liveScaleHint->setPalette(hintPal);

        auto updateScaleFormulaText = [liveScaleHint](int limitVal) {
            int safeLim = std::max(1, limitVal);
            double oneCardPct = 100.0 / static_cast<double>(safeLim);
            liveScaleHint->setText(
                QString("✔ Сейчас установлено: %1 карточек = 100%   (1 карточка = %2% шкалы)")
                .arg(safeLim)
                .arg(oneCardPct, 0, 'f', safeLim > 100 ? 2 : 1)
            );
        };
        updateScaleFormulaText(curTrackerLimit);

        trRoot->addWidget(liveScaleHint);
        trRoot->addStretch(1);

        tabs->addTab(visTab,     "1. Интерфейс и Поля");
        tabs->addTab(editorTab,  "2. Панели и Редактор");
        tabs->addTab(hkTab,      "3. Горячие клавиши");
        tabs->addTab(trackerTab, "4. 📊 Трекер (Шкала)");

        root->addWidget(tabs, 1);

        // ====================================================================
        // НИЖНИЙ РЯД КНОПОК
        // ====================================================================
        QHBoxLayout* btnRow = new QHBoxLayout();
        NeonButton* resetDefaultBtn = new NeonButton("↺ Сбросить по умолчанию (400×600, без поля кода)", QColor("#fb923c"), this);
        NeonButton* closeBtn        = new NeonButton("Отмена", QColor("#94a3b8"), this);
        NeonButton* saveBtn         = new NeonButton("💾 Применить и сохранить", QColor("#4ade80"), this);
        resetDefaultBtn->setMinimumHeight(38);
        closeBtn->setMinimumHeight(38);
        saveBtn->setMinimumHeight(38);

        btnRow->addWidget(resetDefaultBtn);
        btnRow->addStretch(1);
        btnRow->addWidget(closeBtn);
        btnRow->addWidget(saveBtn);
        root->addLayout(btnRow);

        // ====================================================================
        // ДВУСТОРОННЯЯ СИНХРОНИЗАЦИЯ ПОЛЗУНКОВ, ПОЛЕЙ QSPINBOX И ГАЛОЧЕК
        // ====================================================================
        auto bindSliderAndSpin = [](QSlider* sl, QSpinBox* sp) {
            connect(sl, &QSlider::valueChanged, [sp](int v) {
                if (sp->value() != v) sp->setValue(v);
            });
                connect(sp, QOverload<int>::of(&QSpinBox::valueChanged), [sl](int v) {
                    int clamped = std::clamp(v, sl->minimum(), sl->maximum());
                    if (sl->value() != clamped) sl->setValue(clamped);
                });
        };

        bindSliderAndSpin(uiFontSlider, uiFontSpin);
        bindSliderAndSpin(edFontSlider, edFontSpin);
        bindSliderAndSpin(opSlider, opSpin);
        bindSliderAndSpin(blurSlider, blurSpin);
        bindSliderAndSpin(leftWidthSlider, leftWidthSpin);
        bindSliderAndSpin(drawerWidthSlider, drawerWidthSpin);
        bindSliderAndSpin(trackerLimitSlider, trackerLimitSpin);

        connect(trackerLimitSpin, QOverload<int>::of(&QSpinBox::valueChanged), updateScaleFormulaText);

        connect(showCodeFieldCheck1, &QCheckBox::toggled, [showCodeFieldCheck2](bool checked) {
            if (showCodeFieldCheck2->isChecked() != checked) showCodeFieldCheck2->setChecked(checked);
        });
            connect(showCodeFieldCheck2, &QCheckBox::toggled, [showCodeFieldCheck1](bool checked) {
                if (showCodeFieldCheck1->isChecked() != checked) showCodeFieldCheck1->setChecked(checked);
            });

                connect(rememberGeomCheck1, &QCheckBox::toggled, [rememberGeomCheck2](bool checked) {
                    if (rememberGeomCheck2->isChecked() != checked) rememberGeomCheck2->setChecked(checked);
                });
                    connect(rememberGeomCheck2, &QCheckBox::toggled, [rememberGeomCheck1](bool checked) {
                        if (rememberGeomCheck1->isChecked() != checked) rememberGeomCheck1->setChecked(checked);
                    });

                        auto syncCardWidthControls = [=](int v) {
                            if (cardWidthSlider1->value() != v) cardWidthSlider1->setValue(v);
                            if (cardWidthSpin1->value()   != v) cardWidthSpin1->setValue(v);
                            if (cardWidthSlider2->value() != v) cardWidthSlider2->setValue(v);
                            if (cardWidthSpin2->value()   != v) cardWidthSpin2->setValue(v);
                        };
                            connect(cardWidthSlider1, &QSlider::valueChanged, syncCardWidthControls);
                            connect(cardWidthSpin1, QOverload<int>::of(&QSpinBox::valueChanged), syncCardWidthControls);
                            connect(cardWidthSlider2, &QSlider::valueChanged, syncCardWidthControls);
                            connect(cardWidthSpin2, QOverload<int>::of(&QSpinBox::valueChanged), syncCardWidthControls);

                            auto syncCardHeightControls = [=](int v) {
                                if (cardHeightSlider1->value() != v) cardHeightSlider1->setValue(v);
                                if (cardHeightSpin1->value()   != v) cardHeightSpin1->setValue(v);
                                if (cardHeightSlider2->value() != v) cardHeightSlider2->setValue(v);
                                if (cardHeightSpin2->value()   != v) cardHeightSpin2->setValue(v);
                            };
                                connect(cardHeightSlider1, &QSlider::valueChanged, syncCardHeightControls);
                                connect(cardHeightSpin1, QOverload<int>::of(&QSpinBox::valueChanged), syncCardHeightControls);
                                connect(cardHeightSlider2, &QSlider::valueChanged, syncCardHeightControls);
                                connect(cardHeightSpin2, QOverload<int>::of(&QSpinBox::valueChanged), syncCardHeightControls);

                                connect(resetDefaultBtn, &QPushButton::clicked, [=]() {
                                    AppConfig def;
                                    uiFontSpin->setValue(13);
                                    edFontSpin->setValue(14);
                                    syncCardWidthControls(400);
                                    syncCardHeightControls(600);
                                    opSpin->setValue(def.cardOpacity * 100 / 255);
                                    blurSpin->setValue(def.shadowBlurRadius);
                                    showCodeFieldCheck1->setChecked(false);
                                    showTopBarCheck->setChecked(false);
                                    rememberGeomCheck1->setChecked(true);
                                    glassCheck->setChecked(def.enableBlurGlass);
                                    shadowCheck->setChecked(def.enableShadows);
                                    hoverCheck->setChecked(def.enableHoverFloat);
                                    editorModeCombo->setCurrentIndex(def.useRightDrawer ? 0 : 1);
                                    leftWidthSpin->setValue(def.leftPanelWidth);
                                    drawerWidthSpin->setValue(def.rightDrawerWidth);
                                    trackerLimitSpin->setValue(100);

                                    hkInsertCodeEdit->setKeySequence(QKeySequence("Ctrl+K"));
                                    hkFormatPopupEdit->setKeySequence(QKeySequence(def.hkFormatPopup));
                                    hkNextFieldEdit->setKeySequence(QKeySequence("Ctrl+Tab"));
                                    hkNewCardEdit->setKeySequence(QKeySequence(def.hkNewCard));
                                    hkVaultEdit->setKeySequence(QKeySequence(def.hkToggleVault));
                                    hkDrawerEdit->setKeySequence(QKeySequence(def.hkToggleDrawer));
                                    hkTrackerEdit->setKeySequence(QKeySequence(def.hkTracker));
                                    hkSettingsEdit->setKeySequence(QKeySequence(def.hkSettings));
                                    hkSearchEdit->setKeySequence(QKeySequence(def.hkFocusSearch));
                                });

                                connect(closeBtn, &QPushButton::clicked, this, &QDialog::reject);

                                connect(saveBtn, &QPushButton::clicked, [=, &db]() {
                                    CfgBridge::getUiFontSize(g_cfg)         = uiFontSpin->value();
                                    CfgBridge::getEditorFontSize(g_cfg)     = edFontSpin->value();
                                    CfgBridge::getCardFixedWidth()          = cardWidthSpin1->value();
                                    CfgBridge::getCardFixedHeight()         = cardHeightSpin1->value();
                                    CfgBridge::getRememberWindowGeometry()  = rememberGeomCheck1->isChecked();
                                    CfgBridge::syncScaleFromWidth();

                                    g_cfg.cardOpacity         = std::clamp(opSpin->value() * 255 / 100, 80, 255);
                                    g_cfg.shadowBlurRadius    = blurSpin->value();
                                    g_cfg.showCodeField       = showCodeFieldCheck1->isChecked();
                                    g_cfg.showTopSectionBar   = showTopBarCheck->isChecked();
                                    g_cfg.enableBlurGlass     = glassCheck->isChecked();
                                    g_cfg.enableShadows       = shadowCheck->isChecked();
                                    g_cfg.enableHoverFloat    = hoverCheck->isChecked();
                                    g_cfg.useRightDrawer      = editorModeCombo->currentData().toBool();
                                    g_cfg.leftPanelWidth      = leftWidthSpin->value();
                                    g_cfg.rightDrawerWidth    = drawerWidthSpin->value();
                                    g_cfg.trackerCardsForFull = std::max(1, trackerLimitSpin->value());

                                    if (!hkInsertCodeEdit->keySequence().isEmpty()) {
                                        CfgBridge::getHkInsertCode() = hkInsertCodeEdit->keySequence().toString();
                                    }
                                    if (!hkFormatPopupEdit->keySequence().isEmpty()) g_cfg.hkFormatPopup  = hkFormatPopupEdit->keySequence().toString();
                                    if (!hkNextFieldEdit->keySequence().isEmpty())   CfgBridge::getHkNextField(g_cfg) = hkNextFieldEdit->keySequence().toString();
                                    if (!hkNewCardEdit->keySequence().isEmpty())     g_cfg.hkNewCard      = hkNewCardEdit->keySequence().toString();
                                    if (!hkVaultEdit->keySequence().isEmpty())       g_cfg.hkToggleVault  = hkVaultEdit->keySequence().toString();
                                    if (!hkDrawerEdit->keySequence().isEmpty())      g_cfg.hkToggleDrawer = hkDrawerEdit->keySequence().toString();
                                    if (!hkTrackerEdit->keySequence().isEmpty())     g_cfg.hkTracker      = hkTrackerEdit->keySequence().toString();
                                    if (!hkSettingsEdit->keySequence().isEmpty())    g_cfg.hkSettings     = hkSettingsEdit->keySequence().toString();
                                    if (!hkSearchEdit->keySequence().isEmpty())      g_cfg.hkFocusSearch  = hkSearchEdit->keySequence().toString();

                                    db.saveConfig(g_cfg);
                                    CfgBridge::saveExtraSettings();
                                    CfgBridge::applyGlobalFontSizes();
                                    if (onApplyLive) onApplyLive();
                                    accept();
                                });
    }
};
