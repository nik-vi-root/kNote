#pragma once
#include "common.hpp"
#include "right_panel.hpp"
#include "settings.hpp"
#include <QMoveEvent>
#include <QCloseEvent>

// ============================================================================
// 1. ОБЪЕДИНЕННАЯ ЛЕВАЯ ПАНЕЛЬ (Ширина регулируется цифрами в Настройках)
// ============================================================================
class UnifiedLeftPanel : public QFrame {
private:
    fs::path vaultRoot;
    DatabaseManager& db;
    QTreeWidget* fileTree = nullptr;
    bool floatingMode = false;

    std::function<void()> onShowAllChapters;
    std::function<void(std::string, std::string)> onSelectChapterSection;
    std::function<void(Card)> onOpenCardDetails;
    std::function<void()> onStructureModified;

public:
    UnifiedLeftPanel(const fs::path& vaultDir,
                     DatabaseManager& dbRef,
                     std::function<void()> showAllCb,
                     std::function<void(std::string, std::string)> selectChSecCb,
                     std::function<void(Card)> openDetailsCb,
                     std::function<void()> modifiedCb,
                     QWidget* parent = nullptr,
                     bool isFloatingWindow = false)
    : QFrame(parent),
    vaultRoot(vaultDir),
    db(dbRef),
    floatingMode(isFloatingWindow),
    onShowAllChapters(showAllCb),
    onSelectChapterSection(selectChSecCb),
    onOpenCardDetails(openDetailsCb),
    onStructureModified(modifiedCb)
    {
        if (floatingMode) {
            setMinimumWidth(260);
        } else {
            applyConfiguredWidth();
        }

        QVBoxLayout* fmLayout = new QVBoxLayout(this);
        fmLayout->setContentsMargins(14, 16, 14, 16);
        fmLayout->setSpacing(10);

        QFont boldF = font();
        boldF.setBold(true);

        QLabel* fmHeader = new QLabel("📁 ПРОВОДНИК И ГЛАВЫ", this);
        fmHeader->setFont(boldF);

        NeonButton* pinnedOnlyBtn = new NeonButton("⭐ Избранные карточки (по умолч.)", QColor("#fbbf24"), this);
        pinnedOnlyBtn->setMinimumHeight(34);

        NeonButton* allChaptersBtn = new NeonButton("🌐 Показать все главы и разделы", QColor("#38bdf8"), this);
        allChaptersBtn->setMinimumHeight(34);

        QHBoxLayout* fmBtnsRow = new QHBoxLayout();
        NeonButton* fmNewNoteBtn   = new NeonButton("+ Заметка", QColor("#4ade80"), this);
        NeonButton* fmNewFolderBtn = new NeonButton("+ Папка", QColor("#c084fc"), this);
        fmBtnsRow->addWidget(fmNewNoteBtn);
        fmBtnsRow->addWidget(fmNewFolderBtn);

        fileTree = new QTreeWidget(this);
        fileTree->setHeaderHidden(true);
        fileTree->setContextMenuPolicy(Qt::CustomContextMenu);
        fileTree->setIndentation(16);
        fileTree->setAnimated(true);

        NeonInputFrame* treeFrame = new NeonInputFrame(fileTree, QColor("#38bdf8"), this);

        QLabel* fmHint = new QLabel(
            "• По умолчанию открыты только Избранные (★)\n"
            "• Клик по папке или заметке открывает её карточки\n"
            "• ПКМ по папке — создать заметку или подпапку\n"
            "• Двусторонняя синхронизация с Obsidian (.md)",
                                    this
        );
        fmHint->setWordWrap(true);

        fmLayout->addWidget(fmHeader);
        fmLayout->addWidget(pinnedOnlyBtn);
        fmLayout->addWidget(allChaptersBtn);
        fmLayout->addLayout(fmBtnsRow);
        fmLayout->addWidget(treeFrame, 1);
        fmLayout->addWidget(fmHint);

        connect(pinnedOnlyBtn, &QPushButton::clicked, [this]() {
            if (onSelectChapterSection) onSelectChapterSection("__pinned__", "all");
        });

            connect(allChaptersBtn, &QPushButton::clicked, [this]() {
                if (onShowAllChapters) onShowAllChapters();
            });

                connect(fmNewNoteBtn, &QPushButton::clicked, [this]() {
                    fs::path targetDir = getTargetFolderFromTreeItem(fileTree->currentItem());
                    promptCreateNoteInFolder(targetDir);
                });

                connect(fmNewFolderBtn, &QPushButton::clicked, [this]() {
                    fs::path targetDir = getTargetFolderFromTreeItem(fileTree->currentItem());
                    promptCreateFolder(targetDir);
                });

                connect(fileTree, &QTreeWidget::itemClicked, [this](QTreeWidgetItem* item, int) {
                    if (!item) return;
                    std::string type = item->data(0, Qt::UserRole + 1).toString().toStdString();
                    fs::path clickedPath = u8path(item->data(0, Qt::UserRole).toString().toStdString());

                    if (type == "dir") {
                        auto [ch, sec] = extractChapterAndSectionFromDir(clickedPath);
                        if (onSelectChapterSection) onSelectChapterSection(ch, sec);
                    } else if (type == "file") {
                        std::error_code ec;
                        for (auto& c : db.getAllCards()) {
                            if (!c.filePath.empty()) {
                                fs::path cardFsPath = u8path(c.filePath);
                                if (fs::exists(cardFsPath, ec) && fs::exists(clickedPath, ec)) {
                                    if (fs::equivalent(cardFsPath, clickedPath, ec)) {
                                        if (onSelectChapterSection) onSelectChapterSection(c.chapter, c.section);
                                        if (onOpenCardDetails) onOpenCardDetails(c);
                                        return;
                                    }
                                }
                            }
                        }
                    }
                });

                connect(fileTree, &QTreeWidget::customContextMenuRequested, [this](const QPoint& pos) {
                    QTreeWidgetItem* clickedItem = fileTree->itemAt(pos);
                    fs::path targetDir = getTargetFolderFromTreeItem(clickedItem);

                    QMenu menu(this);
                    std::string dirLabel = (targetDir == vaultRoot) ? "корень vault" : filenameToUtf8(targetDir);

                    QAction* actNote = menu.addAction(QString::fromStdString("📄 Создать заметку в «" + dirLabel + "»"));
                    QAction* actDir  = menu.addAction(QString::fromStdString("📁 Создать папку в «" + dirLabel + "»"));
                    QAction* actDel  = nullptr;
                    if (clickedItem) {
                        menu.addSeparator();
                        actDel = menu.addAction("🗑 Удалить выбранный элемент");
                    }

                    QAction* chosen = menu.exec(fileTree->viewport()->mapToGlobal(pos));
                    if (!chosen) return;

                    if (chosen == actNote) {
                        promptCreateNoteInFolder(targetDir);
                    } else if (chosen == actDir) {
                        promptCreateFolder(targetDir);
                    } else if (chosen == actDel && clickedItem) {
                        fs::path delPath = u8path(clickedItem->data(0, Qt::UserRole).toString().toStdString());
                        if (QMessageBox::question(this, "Удаление", "Удалить " + QString::fromStdString(filenameToUtf8(delPath)) + "?") == QMessageBox::Yes) {
                            beginIgnoreFsWatcher(900);
                            std::error_code ec;
                            std::string prefix = pathToUtf8(fs::weakly_canonical(delPath, ec));
                            for (const auto& c : db.getAllCards()) {
                                if (c.filePath.empty()) continue;
                                std::string cp = pathToUtf8(fs::weakly_canonical(u8path(c.filePath), ec));
                                if (cp == prefix || cp.rfind(prefix + "/", 0) == 0) {
                                    db.deleteCardRecordOnly(c.id);
                                }
                            }
                            fs::remove_all(delPath, ec);
                            db.syncDirectoryWithDatabase();
                            if (onStructureModified) onStructureModified();
                        }
                    }
                });
    }

    // Применяет ширину левой панели, заданную цифрами в Настройках
    void applyConfiguredWidth() {
        if (!floatingMode) {
            int w = std::clamp(g_cfg.leftPanelWidth, 200, 800);
            setFixedWidth(w);
        }
    }

    std::pair<std::string, std::string> extractChapterAndSectionFromDir(const fs::path& dirPath) {
        std::error_code ec;
        fs::path rel = fs::relative(dirPath, vaultRoot, ec);
        return DatabaseManager::extractChapterAndSectionFromRelDir(rel);
    }

    fs::path getTargetFolderFromTreeItem(QTreeWidgetItem* item) {
        if (!item) return vaultRoot;
        std::string type = item->data(0, Qt::UserRole + 1).toString().toStdString();
        fs::path p = u8path(item->data(0, Qt::UserRole).toString().toStdString());
        return (type == "dir") ? p : p.parent_path();
    }

    void promptCreateNoteInFolder(const fs::path& targetDir) {
        auto [chapter, section] = extractChapterAndSectionFromDir(targetDir);
        Card newCard;
        newCard.chapter = (chapter == "all") ? "Общее" : chapter;
        newCard.section = (section == "all") ? "Заметки" : section;

        CardEditorDialog dlg(newCard, true, db, this);
        if (dlg.exec() == QDialog::Accepted) {
            if (onStructureModified) onStructureModified();
            if (onSelectChapterSection) {
                onSelectChapterSection(
                    (chapter == "all") ? newCard.chapter : chapter,
                                       (section == "all") ? newCard.section : section
                );
            }
        }
    }

    void promptCreateFolder(const fs::path& targetDir) {
        bool ok = false;
        std::string parentName = (targetDir == vaultRoot) ? "корень vault" : filenameToUtf8(targetDir);
        QString folderName = QInputDialog::getText(this, "Новая папка",
                                                   "Имя папки (внутри «" + QString::fromStdString(parentName) + "»):",
                                                   QLineEdit::Normal, "", &ok);

        if (ok && !folderName.trimmed().isEmpty()) {
            std::error_code ec;
            fs::create_directories(targetDir / u8path(folderName.trimmed().toStdString()), ec);
            if (onStructureModified) onStructureModified();
        }
    }

    void populateTreeRecursive(const fs::path& dir, QTreeWidgetItem* parentItem) {
        std::error_code ec;
        if (!fs::exists(dir, ec)) return;

        std::vector<fs::directory_entry> entries;
        for (const auto& e : fs::directory_iterator(dir, fs::directory_options::skip_permission_denied, ec)) {
            entries.push_back(e);
        }

        std::sort(entries.begin(), entries.end(), [](const fs::directory_entry& a, const fs::directory_entry& b) {
            std::error_code ec1, ec2;
            bool da = a.is_directory(ec1);
            bool db = b.is_directory(ec2);
            if (da != db) return da > db;
            std::string na = filenameToUtf8(a.path());
            std::string nb = filenameToUtf8(b.path());
            int ia = std::atoi(na.c_str());
            int ib = std::atoi(nb.c_str());
            if (ia > 0 && ib > 0 && ia != ib) return ia < ib;
            return na < nb;
        });

        for (const auto& entry : entries) {
            std::string name = filenameToUtf8(entry.path());
            if (!name.empty() && name[0] == '.') continue;
            bool isDir = entry.is_directory(ec);
            if (isDir && (name == "attachments" || name == "images")) continue;
            if (!isDir && !DatabaseManager::isNoteFile(entry.path())) continue;

            QTreeWidgetItem* item = new QTreeWidgetItem();
            QColor neonColor = getModernColorForName(name);
            std::string fullPathUtf8 = pathToUtf8(entry.path());

            if (isDir) {
                item->setText(0, QString::fromStdString("📁  " + name));
                item->setData(0, Qt::UserRole, QString::fromStdString(fullPathUtf8));
                item->setData(0, Qt::UserRole + 1, "dir");

                QFont f = fileTree->font();
                f.setPointSize(g_cfg.uiFontSize);
                f.setBold(true);
                item->setFont(0, f);
                item->setForeground(0, QBrush(neonColor));
            } else {
                std::string stemName = stemToUtf8(entry.path());
                QColor noteColor = getModernColorForName(stemName);
                item->setText(0, QString::fromStdString("📄  " + stemName));
                item->setData(0, Qt::UserRole, QString::fromStdString(fullPathUtf8));
                item->setData(0, Qt::UserRole + 1, "file");
                item->setForeground(0, QBrush(noteColor.lighter(120)));
            }

            if (parentItem) parentItem->addChild(item);
            else fileTree->addTopLevelItem(item);

            if (isDir) {
                populateTreeRecursive(entry.path(), item);
                item->setExpanded(false);
            }
        }
    }

    void refreshTree() {
        fileTree->clear();
        populateTreeRecursive(vaultRoot, nullptr);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.fillRect(rect(), g_cfg.bgLeftPanel);

        if (!floatingMode) {
            QColor borderCol = g_cfg.isMaleTheme ? QColor(56, 189, 248, 150) : QColor(244, 114, 182, 150);
            p.setPen(QPen(borderCol, 1.6));
            p.drawLine(width() - 1, 0, width() - 1, height());
        }
    }
};

// ============================================================================
// 2. ОТДЕЛЬНОЕ НЕМОДАЛЬНОЕ ОКНО ПРОВОДНИКА (Для малого режима главного окна)
// ============================================================================
class FloatingVaultWindow : public QDialog {
private:
    UnifiedLeftPanel* innerPanel = nullptr;

public:
    FloatingVaultWindow(const fs::path& vaultDir,
                        DatabaseManager& dbRef,
                        std::function<void()> showAllCb,
                        std::function<void(std::string, std::string)> selectChSecCb,
                        std::function<void(Card)> openDetailsCb,
                        std::function<void()> modifiedCb,
                        QWidget* parent = nullptr)
    : QDialog(parent, Qt::Window)
    {
        setWindowTitle("📁 Проводник глав и папок");
        setModal(false);
        setWindowModality(Qt::NonModal);
        setMinimumSize(280, 420);

        CfgBridge::restoreWindowRect("leftVaultWindowGeometry", this, QSize(350, 620));

        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(0, 0, 0, 0);
        l->setSpacing(0);

        innerPanel = new UnifiedLeftPanel(
            vaultDir, dbRef, showAllCb, selectChSecCb, openDetailsCb, modifiedCb, this, true
        );
        l->addWidget(innerPanel, 1);
    }

    void refreshTree() {
        if (innerPanel) innerPanel->refreshTree();
    }

protected:
    void moveEvent(QMoveEvent* e) override {
        QDialog::moveEvent(e);
        if (isVisible()) CfgBridge::saveWindowRect("leftVaultWindowGeometry", this);
    }

    void resizeEvent(QResizeEvent* e) override {
        QResizeEvent* re = e;
        QDialog::resizeEvent(re);
        if (isVisible()) CfgBridge::saveWindowRect("leftVaultWindowGeometry", this);
    }

    void closeEvent(QCloseEvent* e) override {
        CfgBridge::saveWindowRect("leftVaultWindowGeometry", this);
        QDialog::closeEvent(e);
    }
};
