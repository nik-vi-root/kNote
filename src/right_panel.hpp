#pragma once
#include "common.hpp"
#include "dock_panel.hpp"
#include "settings.hpp"
#include <QAbstractTextDocumentLayout>
#include <QScrollBar>
#include <QWheelEvent>
#include <QMoveEvent>
#include <QCloseEvent>

// ============================================================================
// 0. БЕЗОПАСНОЕ УВЕЛИЧЕНИЕ КАРТИНОК (Статичное превью + Лайтбокс по клику)
// ============================================================================
class ImageHoverPopup : public QFrame {
private:
    QLabel* imgLabel = nullptr;
    QLabel* hintLabel = nullptr;
    QString loadedPath;
    QPixmap cachedScaledPixmap;

public:
    explicit ImageHoverPopup(QWidget* parent = nullptr)
    : QFrame(parent,
             Qt::ToolTip |
             Qt::FramelessWindowHint |
             Qt::WindowTransparentForInput |
             Qt::WindowDoesNotAcceptFocus)
    {
        setAttribute(Qt::WA_ShowWithoutActivating, true);
        setAttribute(Qt::WA_TransparentForMouseEvents, true);

        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(12, 12, 12, 10);
        l->setSpacing(6);

        imgLabel = new QLabel(this);
        imgLabel->setAlignment(Qt::AlignCenter);

        hintLabel = new QLabel("🔍 Клик по картинке — полноэкранный просмотр и зум колесиком", this);
        hintLabel->setAlignment(Qt::AlignCenter);
        QFont f = font();
        f.setPointSize(9);
        f.setBold(true);
        hintLabel->setFont(f);
        QPalette pal = hintLabel->palette();
        pal.setColor(QPalette::WindowText, QColor("#38bdf8"));
        hintLabel->setPalette(pal);

        l->addWidget(imgLabel);
        l->addWidget(hintLabel);
    }

    void showOnceForImage(const QString& imagePath, const QRect& globalImageRect) {
        if (imagePath.isEmpty()) {
            hide();
            return;
        }

        QScreen* scr = QGuiApplication::screenAt(globalImageRect.center());
        if (!scr) scr = QGuiApplication::primaryScreen();
        QRect avail = scr ? scr->availableGeometry() : QRect(0, 0, 1440, 880);

        if (loadedPath != imagePath || cachedScaledPixmap.isNull()) {
            QPixmap pix(imagePath);
            if (pix.isNull()) {
                hide();
                return;
            }
            loadedPath = imagePath;

            int maxW = std::min(780, static_cast<int>(avail.width() * 0.52));
            int maxH = std::min(600, static_cast<int>(avail.height() * 0.68));

            cachedScaledPixmap = pix.scaled(maxW, maxH, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            imgLabel->setPixmap(cachedScaledPixmap);
            adjustSize();
        }

        int x = globalImageRect.left() - width() - 18;
        if (x < avail.left() + 10) {
            x = globalImageRect.right() + 18;
        }
        if (x + width() > avail.right() - 10) {
            x = std::max(avail.left() + 10, avail.right() - width() - 10);
        }

        int y = globalImageRect.center().y() - height() / 2;
        y = std::clamp(y, avail.top() + 10, avail.bottom() - height() - 10);

        move(x, y);
        if (!isVisible()) {
            show();
            raise();
        }
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(2.0, 2.0, -2.0, -2.0);
        p.setBrush(QColor(11, 19, 41, 248));
        QColor bc = g_cfg.isMaleTheme ? QColor("#38bdf8") : QColor("#f472b6");
        p.setPen(QPen(bc, 2.0));
        p.drawRoundedRect(r, 12, 12);
    }
};

class ImageLightboxDialog : public QDialog {
private:
    QPixmap originalPix;
    QLabel* imgLabel = nullptr;
    QScrollArea* scrollArea = nullptr;
    QLabel* zoomBadge = nullptr;
    double scaleFactor = 1.0;

    void updateScaledPixmap() {
        if (originalPix.isNull()) return;
        int w = std::max(64, static_cast<int>(originalPix.width() * scaleFactor));
        int h = std::max(64, static_cast<int>(originalPix.height() * scaleFactor));
        imgLabel->setPixmap(originalPix.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        imgLabel->resize(w, h);
        if (zoomBadge) {
            zoomBadge->setText(QString("Масштаб: %1%  (Колесо мыши — зум, Клик / Esc — закрыть)")
            .arg(static_cast<int>(std::round(scaleFactor * 100))));
        }
    }

public:
    ImageLightboxDialog(const QString& imagePath, QWidget* parent = nullptr)
    : QDialog(parent, Qt::Dialog | Qt::FramelessWindowHint)
    {
        setModal(true);
        originalPix.load(imagePath);

        QScreen* scr = QGuiApplication::primaryScreen();
        QRect avail = scr ? scr->availableGeometry() : QRect(0, 0, 1440, 880);
        int dlgW = static_cast<int>(avail.width() * 0.82);
        int dlgH = static_cast<int>(avail.height() * 0.84);
        resize(dlgW, dlgH);

        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(16, 14, 16, 14);
        root->setSpacing(10);

        QHBoxLayout* topBar = new QHBoxLayout();
        zoomBadge = new QLabel(this);
        QFont bf = font();
        bf.setBold(true);
        zoomBadge->setFont(bf);

        NeonButton* zoomOutBtn = new NeonButton("−", QColor("#38bdf8"), this);
        NeonButton* zoom100Btn = new NeonButton("100%", QColor("#a855f7"), this);
        NeonButton* zoomInBtn  = new NeonButton("+", QColor("#38bdf8"), this);
        NeonButton* closeBtn   = new NeonButton("× Закрыть", QColor("#fb7185"), this);
        zoomOutBtn->setFixedSize(36, 32);
        zoom100Btn->setFixedHeight(32);
        zoomInBtn->setFixedSize(36, 32);
        closeBtn->setFixedHeight(32);

        topBar->addWidget(zoomBadge, 1);
        topBar->addWidget(zoomOutBtn);
        topBar->addWidget(zoom100Btn);
        topBar->addWidget(zoomInBtn);
        topBar->addWidget(closeBtn);

        scrollArea = new QScrollArea(this);
        scrollArea->setWidgetResizable(false);
        scrollArea->setAlignment(Qt::AlignCenter);
        scrollArea->setFrameShape(QFrame::NoFrame);
        scrollArea->viewport()->setAutoFillBackground(false);

        imgLabel = new QLabel(scrollArea);
        imgLabel->setAlignment(Qt::AlignCenter);
        imgLabel->setCursor(Qt::PointingHandCursor);
        scrollArea->setWidget(imgLabel);

        root->addLayout(topBar);
        root->addWidget(scrollArea, 1);

        if (!originalPix.isNull()) {
            double fitW = static_cast<double>(dlgW - 60) / std::max(1, originalPix.width());
            double fitH = static_cast<double>(dlgH - 90) / std::max(1, originalPix.height());
            scaleFactor = std::clamp(std::min(fitW, fitH), 0.25, 2.2);
            updateScaledPixmap();
        }

        connect(zoomInBtn,  &QPushButton::clicked, [this]() { scaleFactor = std::min(4.0, scaleFactor * 1.2); updateScaledPixmap(); });
        connect(zoomOutBtn, &QPushButton::clicked, [this]() { scaleFactor = std::max(0.2, scaleFactor / 1.2); updateScaledPixmap(); });
        connect(zoom100Btn, &QPushButton::clicked, [this]() { scaleFactor = 1.0; updateScaledPixmap(); });
        connect(closeBtn,   &QPushButton::clicked, this, &QDialog::accept);
    }

protected:
    void wheelEvent(QWheelEvent* e) override {
        if (e->angleDelta().y() > 0) {
            scaleFactor = std::min(4.0, scaleFactor * 1.15);
        } else if (e->angleDelta().y() < 0) {
            scaleFactor = std::max(0.2, scaleFactor / 1.15);
        }
        updateScaledPixmap();
        e->accept();
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            accept();
            return;
        }
        QDialog::mousePressEvent(e);
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
        p.setBrush(QColor(8, 14, 30, 245));
        QColor bc = g_cfg.isMaleTheme ? QColor("#38bdf8") : QColor("#f472b6");
        p.setPen(QPen(bc, 2.0));
        p.drawRoundedRect(r, 14, 14);
    }
};

// ============================================================================
// 1. ПОЛЕ ОПИСАНИЯ С ПОДДЕРЖКОЙ ПОТОКОВЫХ ВЛОЖЕНИЙ И БЛОКОВ OBSIDIAN
//    - Файлы вставляются прямо в текст (как [[файл.ext]] в Obsidian)
//    - Если файл удалён с диска — в потоке текста горит красным [⚠ Файл удалён: ...]
//    - Отрисовка кодовых блоков ```lang ... ```, Callout-блоков, цитат и чекбоксов
// ============================================================================
class ImagePasteTextEdit : public QTextEdit {
private:
    inline static ImageHoverPopup* s_hoverPopup = nullptr;
    QPoint pressPos;

    QString activeHoverImgPath;
    QRect   activeHoverImgVpRect;
    QTimer* hoverWatchTimer = nullptr;

    static ImageHoverPopup* hoverPopupInstance() {
        if (!s_hoverPopup) {
            s_hoverPopup = new ImageHoverPopup();
        }
        return s_hoverPopup;
    }

    void clearHoverImage() {
        activeHoverImgPath.clear();
        activeHoverImgVpRect = QRect();
        if (hoverWatchTimer && hoverWatchTimer->isActive()) {
            hoverWatchTimer->stop();
        }
        if (viewport()) {
            viewport()->setCursor(Qt::IBeamCursor);
        }
        if (s_hoverPopup && s_hoverPopup->isVisible()) {
            s_hoverPopup->hide();
        }
    }

    bool findImageAtViewportPos(const QPoint& vpPos, QString& outPath, QRect& outImgRect) const {
        if (!document() || !document()->documentLayout() || !viewport()) return false;

        int scrollY = verticalScrollBar() ? verticalScrollBar()->value() : 0;
        QAbstractTextDocumentLayout* docLayout = document()->documentLayout();

        for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
            if (!block.isVisible()) continue;
            QRectF blockRect = docLayout->blockBoundingRect(block).translated(0, -scrollY);
            if (blockRect.bottom() < -20 || blockRect.top() > viewport()->height() + 20) continue;

            for (auto it = block.begin(); !it.atEnd(); ++it) {
                QTextFragment frag = it.fragment();
                if (!frag.isValid()) continue;

                QTextCharFormat fmt = frag.charFormat();
                if (!fmt.isImageFormat()) continue;

                QTextImageFormat imgFmt = fmt.toImageFormat();
                QString path = imgFmt.name();
                if (path.isEmpty()) continue;

                QTextCursor cStart(document());
                cStart.setPosition(frag.position());
                QRect rStart = cursorRect(cStart);

                QTextCursor cEnd(document());
                cEnd.setPosition(frag.position() + frag.length());
                QRect rEnd = cursorRect(cEnd);

                int w = (imgFmt.width() > 1.0)
                ? static_cast<int>(imgFmt.width())
                : std::max(40, rEnd.left() - rStart.left());
                int h = (imgFmt.height() > 1.0)
                ? static_cast<int>(imgFmt.height())
                : std::max(40, rStart.height());

                int left   = rStart.left();
                int bottom = rStart.bottom();
                int top    = std::min(rStart.top(), bottom - h);

                QRect imgRect(left, top, w, bottom - top);
                if (imgRect.adjusted(-4, -4, 4, 4).contains(vpPos)) {
                    outPath    = path;
                    outImgRect = imgRect;
                    return true;
                }
            }
        }
        return false;
    }

    void paintObsidianBlocks() {
        if (!document() || !document()->documentLayout() || !viewport()) return;

        QPainter p(viewport());
        p.setRenderHint(QPainter::Antialiasing, true);

        QAbstractTextDocumentLayout* docLayout = document()->documentLayout();
        int scrollY = verticalScrollBar() ? verticalScrollBar()->value() : 0;
        qreal fullLeft  = 4.0;
        qreal fullWidth = std::max(40, viewport()->width() - 8);

        bool inFence = false;
        qreal blockTop = 0.0;
        qreal blockBottom = 0.0;
        QString fenceLang;

        auto drawCodeBox = [&](qreal topY, qreal bottomY, const QString& langLabel) {
            if (bottomY < 0.0 || topY > viewport()->height()) return;
            QRectF boxRect(fullLeft, topY - 2.0, fullWidth, std::max<qreal>(22.0, bottomY - topY + 4.0));

            QPainterPath path;
            path.addRoundedRect(boxRect, 8.0, 8.0);
            p.fillPath(path, QColor("#0b1329"));

            QColor borderCol = g_cfg.isMaleTheme ? QColor(56, 189, 248, 115) : QColor(244, 114, 182, 115);
            p.setPen(QPen(borderCol, 1.3));
            p.drawPath(path);

            if (!langLabel.isEmpty()) {
                QFont lf = font();
                lf.setFamily(MONO_FONT_FAMILY);
                lf.setPointSize(8);
                lf.setBold(true);
                p.setFont(lf);
                p.setPen(QColor("#38bdf8"));
                QRectF badgeRect(boxRect.right() - 120, boxRect.top() + 3, 112, 16);
                p.drawText(badgeRect, Qt::AlignRight | Qt::AlignVCenter, langLabel.toUpper());
            }
        };

        auto drawQuoteOrCalloutLine = [&](const QRectF& br, const QColor& accentCol, bool isHeaderLine) {
            if (br.bottom() < 0.0 || br.top() > viewport()->height()) return;
            QRectF boxRect(fullLeft + 2.0, br.top() - 1.0, fullWidth - 4.0, br.height() + 2.0);
            QColor bg = accentCol;
            bg.setAlpha(isHeaderLine ? 42 : 22);
            p.fillRect(boxRect, bg);

            QRectF barRect(fullLeft + 2.0, br.top() - 1.0, 3.5, br.height() + 2.0);
            p.fillRect(barRect, accentCol);
        };

        QColor currentCalloutAccent;

        for (QTextBlock block = document()->begin(); block.isValid(); block = block.next()) {
            if (!block.isVisible()) continue;

            QRectF br = docLayout->blockBoundingRect(block).translated(0, -scrollY);
            QString trimmed = block.text().trimmed();

            if (trimmed.startsWith("```")) {
                if (!inFence && trimmed.length() > 6 && trimmed.endsWith("```")) {
                    drawCodeBox(br.top(), br.bottom(), QString());
                    continue;
                }

                if (!inFence) {
                    inFence = true;
                    blockTop = br.top();
                    blockBottom = br.bottom();
                    fenceLang = trimmed.mid(3).trimmed();
                } else {
                    blockBottom = br.bottom();
                    drawCodeBox(blockTop, blockBottom, fenceLang);
                    inFence = false;
                    fenceLang.clear();
                }
                continue;
            }

            if (inFence) {
                blockBottom = br.bottom();
                continue;
            }

            if (trimmed.startsWith(">")) {
                QTextCharFormat firstFmt;
                for (auto it = block.begin(); !it.atEnd(); ++it) {
                    if (it.fragment().isValid()) {
                        firstFmt = it.fragment().charFormat();
                        break;
                    }
                }
                QString rawTok = firstFmt.property(PropRawMarkdownToken).toString();
                if (rawTok.contains("[!")) {
                    currentCalloutAccent = firstFmt.foreground().color();
                    if (!currentCalloutAccent.isValid()) currentCalloutAccent = QColor("#38bdf8");
                    drawQuoteOrCalloutLine(br, currentCalloutAccent, true);
                } else {
                    QColor c = currentCalloutAccent.isValid() ? currentCalloutAccent : QColor("#64748b");
                    drawQuoteOrCalloutLine(br, c, false);
                }
            } else {
                currentCalloutAccent = QColor();
            }
        }

        if (inFence) {
            drawCodeBox(blockTop, blockBottom, fenceLang);
        }
    }

public:
    ImagePasteTextEdit(QWidget* parent = nullptr) : QTextEdit(parent) {
        setMouseTracking(true);
        if (viewport()) {
            viewport()->setMouseTracking(true);
        }

        hoverWatchTimer = new QTimer(this);
        hoverWatchTimer->setInterval(60);
        connect(hoverWatchTimer, &QTimer::timeout, this, [this]() {
            if (activeHoverImgPath.isEmpty() || !viewport() || !isVisible()) {
                clearHoverImage();
                return;
            }
            QPoint vpPos = viewport()->mapFromGlobal(QCursor::pos());
            if (!viewport()->rect().contains(vpPos) ||
                !activeHoverImgVpRect.adjusted(-6, -6, 6, 6).contains(vpPos)) {
                clearHoverImage();
                }
        });

        if (verticalScrollBar()) {
            connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int) {
                clearHoverImage();
            });
        }

        connect(document(), &QTextDocument::contentsChanged, this, [this]() {
            if (viewport()) viewport()->update();
        });
    }

    ~ImagePasteTextEdit() override {
        if (g_activeRichEditor == this) {
            g_activeRichEditor = nullptr;
        }
        clearHoverImage();
    }

    void promptAndInsertAttachmentInStream() {
        QStringList files = QFileDialog::getOpenFileNames(
            this,
            "Выберите файлы для вставки в поток текста (как в Obsidian)",
                                                          "",
                                                          "Все файлы (*.*)"
        );
        if (files.isEmpty()) return;

        QTextCursor cursor = textCursor();
        for (const QString& f : files) {
            std::string savedName = AttachmentStorage::attachFileToVault(f);
            if (!savedName.empty()) {
                QString qName = QString::fromStdString(savedName);
                bool isImg = AttachmentStorage::isImageExtension(qName);
                MarkdownCodec::insertFileEmbedAtCursor(cursor, qName, isImg);
            }
        }
        setTextCursor(cursor);
        setFocus();
    }

    void refreshAttachmentLinksInStream() {
        int scrollPos = verticalScrollBar() ? verticalScrollBar()->value() : 0;
        int curPos = textCursor().position();
        std::string serialized = RichTextCodec::serialize(document());

        bool prevBlocked = blockSignals(true);
        RichTextCodec::deserialize(serialized, this, 380);
        blockSignals(prevBlocked);

        QTextCursor c(document());
        c.setPosition(std::min(curPos, std::max(0, document()->characterCount() - 1)));
        setTextCursor(c);
        if (verticalScrollBar()) verticalScrollBar()->setValue(scrollPos);
        if (viewport()) viewport()->update();
    }

protected:
    void paintEvent(QPaintEvent* e) override {
        paintObsidianBlocks();
        QTextEdit::paintEvent(e);
    }

    void keyPressEvent(QKeyEvent* e) override {
        clearHoverImage();
        if (e->key() == Qt::Key_Tab && e->modifiers() == Qt::NoModifier) {
            insertPlainText("  ");
            e->accept();
            return;
        }
        if (e->key() == Qt::Key_Backtab) {
            QTextCursor c = textCursor();
            c.movePosition(QTextCursor::StartOfBlock);
            c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
            QString startTxt = c.selectedText();
            if (startTxt == "  ") {
                c.removeSelectedText();
            } else if (startTxt.startsWith(" ")) {
                c.setPosition(c.block().position());
                c.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 1);
                c.removeSelectedText();
            }
            e->accept();
            return;
        }
        QTextEdit::keyPressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (!activeHoverImgPath.isEmpty() &&
            activeHoverImgVpRect.adjusted(-6, -6, 6, 6).contains(e->pos())) {
            QTextEdit::mouseMoveEvent(e);
        return;
            }

            QString hitPath;
            QRect   hitRect;
            if (findImageAtViewportPos(e->pos(), hitPath, hitRect)) {
                std::error_code ec;
                if (!fs::exists(u8path(hitPath.toStdString()), ec)) {
                    refreshAttachmentLinksInStream();
                    return;
                }
                activeHoverImgPath   = hitPath;
                activeHoverImgVpRect = hitRect;
                viewport()->setCursor(Qt::PointingHandCursor);

                QRect globalImgRect(viewport()->mapToGlobal(hitRect.topLeft()), hitRect.size());
                hoverPopupInstance()->showOnceForImage(hitPath, globalImgRect);
                if (!hoverWatchTimer->isActive()) {
                    hoverWatchTimer->start();
                }
                QTextEdit::mouseMoveEvent(e);
                return;
            } else if (!activeHoverImgPath.isEmpty()) {
                clearHoverImage();
            }

            QString anchor = anchorAt(e->pos());
            if (!anchor.isEmpty() && viewport()) {
                viewport()->setCursor(Qt::PointingHandCursor);
            } else if (viewport()) {
                viewport()->setCursor(Qt::IBeamCursor);
            }

            QTextEdit::mouseMoveEvent(e);
    }

    void leaveEvent(QEvent* e) override {
        if (!activeHoverImgPath.isEmpty() && viewport()) {
            QPoint vpPos = viewport()->mapFromGlobal(QCursor::pos());
            if (viewport()->rect().contains(vpPos) &&
                activeHoverImgVpRect.adjusted(-6, -6, 6, 6).contains(vpPos)) {
                QTextEdit::leaveEvent(e);
            return;
                }
        }
        clearHoverImage();
        QTextEdit::leaveEvent(e);
    }

    void hideEvent(QHideEvent* e) override {
        clearHoverImage();
        QTextEdit::hideEvent(e);
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            pressPos = e->pos();
        }
        clearHoverImage();
        QTextEdit::mousePressEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton && (e->pos() - pressPos).manhattanLength() < 8) {
            QString imgPath;
            QRect   imgRect;
            if (findImageAtViewportPos(e->pos(), imgPath, imgRect)) {
                clearHoverImage();
                std::error_code ec;
                if (!fs::exists(u8path(imgPath.toStdString()), ec)) {
                    refreshAttachmentLinksInStream();
                    e->accept();
                    return;
                }
                ImageLightboxDialog dlg(imgPath, window());
                dlg.exec();
                e->accept();
                return;
            }

            QString href = anchorAt(e->pos());
            if (!href.isEmpty()) {
                QUrl url(href);
                if (url.isLocalFile()) {
                    QString localFile = url.toLocalFile();
                    std::error_code ec;
                    if (!fs::exists(u8path(localFile.toStdString()), ec)) {
                        refreshAttachmentLinksInStream();
                    } else {
                        QDesktopServices::openUrl(url);
                    }
                } else if (url.scheme() != "obsidian") {
                    QDesktopServices::openUrl(url);
                }
                e->accept();
                return;
            }

            QTextCursor c = cursorForPosition(e->pos());
            QTextBlock b = c.block();
            QString bText = b.text();
            if ((bText.startsWith("☐ ") || bText.startsWith("☑ ")) && c.positionInBlock() <= 2) {
                bool nowDone = bText.startsWith("☐ ");
                QTextCursor editCur(b);
                editCur.movePosition(QTextCursor::Right, QTextCursor::KeepAnchor, 2);
                QTextCharFormat cbFmt = editCur.charFormat();
                if (nowDone) cbFmt.setForeground(QColor("#4ade80"));
                editCur.insertText(nowDone ? "☑ " : "☐ ", cbFmt);
                e->accept();
                return;
            }
        }
        QTextEdit::mouseReleaseEvent(e);
    }

    void focusInEvent(QFocusEvent* e) override {
        QTextEdit::focusInEvent(e);
        g_activeRichEditor = this;
    }

    bool canInsertFromMimeData(const QMimeData* source) const override {
        return source->hasImage() || source->hasUrls() || QTextEdit::canInsertFromMimeData(source);
    }

    void insertFromMimeData(const QMimeData* source) override {
        if (source->hasImage()) {
            QImage img = qvariant_cast<QImage>(source->imageData());
            QString savedPath = ImageStorage::saveQImage(img);
            if (!savedPath.isEmpty()) {
                QTextCursor cursor = this->textCursor();
                ImageStorage::insertImageAtCursor(cursor, savedPath, 380);
                return;
            }
        }

        if (source->hasUrls()) {
            QTextCursor cursor = this->textCursor();
            bool handled = false;
            for (const QUrl& url : source->urls()) {
                if (url.isLocalFile()) {
                    QString localPath = url.toLocalFile();
                    std::string savedName = AttachmentStorage::attachFileToVault(localPath);
                    if (!savedName.empty()) {
                        QString qName = QString::fromStdString(savedName);
                        bool isImg = AttachmentStorage::isImageExtension(qName);
                        MarkdownCodec::insertFileEmbedAtCursor(cursor, qName, isImg);
                        handled = true;
                    }
                }
            }
            if (handled) {
                setTextCursor(cursor);
                return;
            }
        }
        QTextEdit::insertFromMimeData(source);
    }
};

// ============================================================================
// 2. РЕДАКТОР С ПАНЕЛЬЮ ФОРМАТИРОВАНИЯ
// ============================================================================
class RichTextEditor : public QWidget {
private:
    ImagePasteTextEdit* editor = nullptr;
    FormattingToolbarPanel* toolbar = nullptr;

public:
    RichTextEditor(QWidget* parent = nullptr, int minHeight = 140, bool includeOwnToolbar = true) : QWidget(parent) {
        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(0, 0, 0, 0);
        root->setSpacing(8);

        editor = new ImagePasteTextEdit(this);
        editor->setMinimumHeight(minHeight);
        editor->setPlaceholderText("Введите текст, блоки кода ``` ... ``` или перетащите файлы (вставляются в поток текста как [[файл]])...");
        QFont f = editor->font();
        f.setPointSize(11);
        editor->setFont(f);

        if (includeOwnToolbar) {
            toolbar = new FormattingToolbarPanel(editor, this);
            root->addWidget(toolbar);
        }

        NeonInputFrame* editorBox = new NeonInputFrame(editor, QColor("#38bdf8"), this);
        root->addWidget(editorBox, 1);
    }

    ImagePasteTextEdit* rawEditor() const { return editor; }

    std::string getSerializedContent() const {
        return RichTextCodec::serialize(editor->document());
    }

    void setSerializedContent(const std::string& data) {
        editor->blockSignals(true);
        RichTextCodec::deserialize(data, editor, 420);
        editor->blockSignals(false);
    }
};

// ============================================================================
// 3. БОКОВАЯ ПРАВАЯ ПАНЕЛЬ «ПОДРОБНЕЕ»
//    Хранит и редактирует отдельно секцию «## Подробнее» (extendedDesc)
// ============================================================================
class RightDrawerPanel : public QFrame {
private:
    QLabel* titleLabel = nullptr;
    RichTextEditor* richEditor = nullptr;
    Card activeCard;
    std::string lastSavedExt;
    QTimer* autoSaveTimer = nullptr;

public:
    RightDrawerPanel(DatabaseManager& db, std::function<void()> onSaved, QWidget* parent = nullptr)
    : QFrame(parent)
    {
        applyConfiguredWidth();

        QVBoxLayout* rdLayout = new QVBoxLayout(this);
        rdLayout->setContentsMargins(16, 16, 16, 16);
        rdLayout->setSpacing(10);

        QFont boldF = font();
        boldF.setBold(true);

        QHBoxLayout* rdHeader = new QHBoxLayout();
        titleLabel = new QLabel("📝 Правая панель: Подробнее", this);
        titleLabel->setFont(boldF);
        titleLabel->setWordWrap(true);
        NeonButton* rdCloseBtn = new NeonButton("×", QColor("#fb7185"), this);
        rdCloseBtn->setFixedSize(32, 32);
        rdHeader->addWidget(titleLabel, 1);
        rdHeader->addWidget(rdCloseBtn);

        richEditor = new RichTextEditor(this, 240, true);

        QHBoxLayout* rdFooter = new QHBoxLayout();
        NeonButton* rdAttachBtn = new NeonButton("📎 Прикрепить файл", QColor("#38bdf8"), this);
        NeonButton* rdCancelBtn = new NeonButton("Скрыть", QColor("#94a3b8"), this);
        NeonButton* rdSaveBtn   = new NeonButton("💾 Сохранить", QColor("#4ade80"), this);
        rdAttachBtn->setMinimumHeight(36);
        rdCancelBtn->setMinimumHeight(36);
        rdSaveBtn->setMinimumHeight(36);

        rdFooter->addWidget(rdAttachBtn);
        rdFooter->addWidget(rdCancelBtn);
        rdFooter->addWidget(rdSaveBtn, 1);

        rdLayout->addLayout(rdHeader);
        rdLayout->addWidget(richEditor, 1);
        rdLayout->addLayout(rdFooter);

        autoSaveTimer = new QTimer(this);
        autoSaveTimer->setSingleShot(true);
        autoSaveTimer->setInterval(550);

        auto saveNow = [this, &db](bool forceSync) {
            if (activeCard.id == 0) return false;
            std::string curExt = richEditor->getSerializedContent();
            if (!forceSync && curExt == lastSavedExt) return false;
            lastSavedExt = curExt;

            for (const auto& c : db.getAllCards()) {
                if (c.id == activeCard.id) {
                    activeCard = c;
                    break;
                }
            }
            activeCard.extendedDesc = curExt;
            db.saveCard(activeCard, true);
            return true;
        };

        connect(rdAttachBtn, &QPushButton::clicked, [this]() {
            if (richEditor && richEditor->rawEditor()) {
                richEditor->rawEditor()->promptAndInsertAttachmentInStream();
            }
        });

        connect(autoSaveTimer, &QTimer::timeout, [saveNow]() {
            saveNow(false);
        });

        connect(richEditor->rawEditor(), &QTextEdit::textChanged, [this]() {
            if (activeCard.id != 0 && richEditor->getSerializedContent() != lastSavedExt) {
                autoSaveTimer->start();
            }
        });

        connect(rdCloseBtn,  &QPushButton::clicked, [this, saveNow, onSaved]() {
            saveNow(false);
            setVisible(false);
            if (onSaved) onSaved();
        });
            connect(rdCancelBtn, &QPushButton::clicked, [this, saveNow, onSaved]() {
                saveNow(false);
                setVisible(false);
                if (onSaved) onSaved();
            });
                connect(rdSaveBtn,   &QPushButton::clicked, [this, saveNow, rdSaveBtn, onSaved]() {
                    saveNow(true);
                    rdSaveBtn->setText("✔ Сохранено!");
                    QPointer<NeonButton> safeBtn(rdSaveBtn);
                    QTimer::singleShot(900, [safeBtn]() {
                        if (safeBtn) safeBtn->setText("💾 Сохранить");
                    });
                        if (onSaved) onSaved();
                });
    }

    void applyConfiguredWidth() {
        int w = std::clamp(g_cfg.rightDrawerWidth, 240, 950);
        setFixedWidth(w);
    }

    void loadCard(const Card& c) {
        activeCard = c;
        titleLabel->setText(QString::fromStdString("📝 " + c.title + " [" + c.chapter + "]"));
        richEditor->setSerializedContent(c.extendedDesc);
        lastSavedExt = richEditor->getSerializedContent();
    }

    void refreshAttachmentsStatus() {
        if (richEditor && richEditor->rawEditor()) {
            richEditor->rawEditor()->refreshAttachmentLinksInStream();
        }
    }

    bool hasCard() const { return activeCard.id != 0; }
    long long currentCardId() const { return activeCard.id; }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        p.fillRect(rect(), g_cfg.bgRightPanel);

        QColor borderCol = g_cfg.isMaleTheme ? QColor(56, 189, 248, 160) : QColor(244, 114, 182, 160);
        p.setPen(QPen(borderCol, 1.8));
        p.drawLine(0, 0, 0, height());
    }
};

// ============================================================================
// 4. НЕМОДАЛЬНОЕ ОКНО «ПОДРОБНЕЕ»
// ============================================================================
class DetailsEditorDialog : public QDialog {
private:
    DatabaseManager& db;
    Card activeCard;
    QLabel* headerLbl = nullptr;
    RichTextEditor* richEditor = nullptr;
    NeonButton* saveBtn = nullptr;
    std::string lastSavedExt;
    QTimer* autoSaveTimer = nullptr;
    std::function<void()> onSavedCallback;

    void saveChangesToDb(bool forceSync) {
        if (activeCard.id == 0) return;
        std::string curExt = richEditor->getSerializedContent();
        if (!forceSync && curExt == lastSavedExt) return;
        lastSavedExt = curExt;

        for (const auto& c : db.getAllCards()) {
            if (c.id == activeCard.id) {
                activeCard = c;
                break;
            }
        }
        activeCard.extendedDesc = curExt;
        db.saveCard(activeCard, true);
    }

public:
    DetailsEditorDialog(const Card& initialCard,
                        DatabaseManager& dbRef,
                        QWidget* parent = nullptr,
                        std::function<void()> onSaved = nullptr)
    : QDialog(parent, Qt::Window), db(dbRef), activeCard(initialCard), onSavedCallback(onSaved)
    {
        setModal(false);
        setWindowModality(Qt::NonModal);
        setMinimumSize(420, 360);

        CfgBridge::restoreWindowRect("detailsWindowGeometry", this, QSize(820, 580));

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setContentsMargins(16, 14, 16, 14);
        layout->setSpacing(10);

        headerLbl = new QLabel(this);
        QFont f = font();
        f.setPointSize(g_cfg.uiFontSize + 1);
        f.setBold(true);
        headerLbl->setFont(f);
        headerLbl->setWordWrap(true);

        richEditor = new RichTextEditor(this, 260, true);

        QHBoxLayout* btnRow = new QHBoxLayout();
        NeonButton* attachBtn = new NeonButton("📎 Прикрепить файл в текст", QColor("#38bdf8"), this);
        QLabel* syncHint      = new QLabel("⚡ Секция «## Подробнее» (отдельно от карточки)", this);
        NeonButton* closeBtn  = new NeonButton("Закрыть окно", QColor("#94a3b8"), this);
        saveBtn               = new NeonButton("💾 Сохранить", QColor("#4ade80"), this);
        attachBtn->setMinimumHeight(36);
        closeBtn->setMinimumHeight(36);
        saveBtn->setMinimumHeight(36);

        btnRow->addWidget(attachBtn);
        btnRow->addWidget(syncHint, 1);
        btnRow->addWidget(closeBtn);
        btnRow->addWidget(saveBtn);

        layout->addWidget(headerLbl);
        layout->addWidget(richEditor, 1);
        layout->addLayout(btnRow);

        autoSaveTimer = new QTimer(this);
        autoSaveTimer->setSingleShot(true);
        autoSaveTimer->setInterval(500);

        connect(attachBtn, &QPushButton::clicked, [this]() {
            if (richEditor && richEditor->rawEditor()) {
                richEditor->rawEditor()->promptAndInsertAttachmentInStream();
            }
        });

        connect(autoSaveTimer, &QTimer::timeout, [this]() {
            saveChangesToDb(false);
        });

        connect(richEditor->rawEditor(), &QTextEdit::textChanged, [this]() {
            if (activeCard.id != 0 && richEditor->getSerializedContent() != lastSavedExt) {
                autoSaveTimer->start();
            }
        });

        connect(closeBtn, &QPushButton::clicked, [this]() {
            saveChangesToDb(false);
            CfgBridge::saveWindowRect("detailsWindowGeometry", this);
            close();
        });

        connect(saveBtn, &QPushButton::clicked, [this]() {
            saveChangesToDb(true);
            CfgBridge::saveWindowRect("detailsWindowGeometry", this);
            saveBtn->setText("✔ Сохранено!");
            QPointer<NeonButton> safeBtn(saveBtn);
            QTimer::singleShot(900, [safeBtn]() {
                if (safeBtn) safeBtn->setText("💾 Сохранить");
            });
                if (onSavedCallback) onSavedCallback();
        });

            loadCard(initialCard);
    }

    void loadCard(const Card& card) {
        if (activeCard.id != 0 && activeCard.id != card.id) {
            saveChangesToDb(false);
        }
        activeCard = card;
        setWindowTitle("Подробное описание • " + QString::fromStdString(card.title));
        headerLbl->setText(QString::fromStdString("📝 " + card.title + "  [" + card.chapter + " • " + card.section + "]"));
        richEditor->setSerializedContent(card.extendedDesc);
        lastSavedExt = richEditor->getSerializedContent();
    }

    void refreshAttachmentsStatus() {
        if (richEditor && richEditor->rawEditor()) {
            richEditor->rawEditor()->refreshAttachmentLinksInStream();
        }
    }

protected:
    void moveEvent(QMoveEvent* e) override {
        QDialog::moveEvent(e);
        if (isVisible()) CfgBridge::saveWindowRect("detailsWindowGeometry", this);
    }

    void resizeEvent(QResizeEvent* e) override {
        QDialog::resizeEvent(e);
        if (isVisible()) CfgBridge::saveWindowRect("detailsWindowGeometry", this);
    }

    void closeEvent(QCloseEvent* e) override {
        saveChangesToDb(false);
        CfgBridge::saveWindowRect("detailsWindowGeometry", this);
        if (onSavedCallback) onSavedCallback();
        QDialog::closeEvent(e);
    }

    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.fillRect(rect(), g_cfg.bgRightPanel);
    }
};

// ============================================================================
// 5. ОКНО СОЗДАНИЯ НОВОЙ КАРТОЧКИ
//    - Лишнее поле «Код / команда» по умолчанию скрыто (включается в Настройках)
//    - Чёткое разделение на 2 вкладки:
//      1) Описание на карточке (с блоками ``` ```)
//      2) Правая панель («Подробнее» / ## Подробнее)
// ============================================================================
class CardEditorDialog : public QDialog {
public:
    CardEditorDialog(Card card, bool isNew, DatabaseManager& db, QWidget* parent = nullptr) : QDialog(parent) {
        setWindowTitle(isNew ? "Новая карточка / заметка" : "Редактирование карточки");
        resize(880, 700);

        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(10);

        QGridLayout* metaGrid = new QGridLayout();
        metaGrid->addWidget(new QLabel("Глава (папка):", this), 0, 0);
        metaGrid->addWidget(new QLabel("Раздел / Подпапка:", this), 0, 1);

        QComboBox* chapterCombo = new QComboBox(this);
        chapterCombo->setEditable(true);
        chapterCombo->setMinimumHeight(34);

        QComboBox* sectionCombo = new QComboBox(this);
        sectionCombo->setEditable(true);
        sectionCombo->setMinimumHeight(34);

        auto existingChapters = db.getExistingChapters();
        for (const auto& ch : existingChapters) {
            chapterCombo->addItem(QString::fromStdString(ch));
        }

        if ((card.chapter == "2) Java script" || card.chapter == "__pinned__" || card.chapter == "all") &&
            std::find(existingChapters.begin(), existingChapters.end(), card.chapter) == existingChapters.end()) {
            card.chapter = existingChapters.empty() ? "Общее" : existingChapters.front();
        if (card.section == "1. Введение" || card.section == "all") card.section = "Заметки";
            }

            auto updateTopicsList = [&db, sectionCombo](const QString& courseText) {
                QString prevSec = sectionCombo->currentText();
                sectionCombo->clear();
                for (const auto& sec : db.getExistingSections(courseText.toStdString())) {
                    sectionCombo->addItem(QString::fromStdString(sec));
                }
                if (!prevSec.isEmpty()) sectionCombo->setCurrentText(prevSec);
            };

                connect(chapterCombo, &QComboBox::currentTextChanged, updateTopicsList);

                std::string initialCh = card.chapter.empty()
                ? (existingChapters.empty() ? "Общее" : existingChapters.front())
                : card.chapter;
                chapterCombo->setCurrentText(QString::fromStdString(initialCh));
                updateTopicsList(chapterCombo->currentText());

                if (!card.section.empty()) {
                    sectionCombo->setCurrentText(QString::fromStdString(card.section));
                } else if (sectionCombo->count() == 0) {
                    sectionCombo->setCurrentText("Заметки");
                }

                metaGrid->addWidget(chapterCombo, 1, 0);
                metaGrid->addWidget(sectionCombo, 1, 1);

                QLabel* titleLbl = new QLabel("Заголовок карточки (имя заметки):", this);
                QLineEdit* titleEdit = new QLineEdit(QString::fromStdString(card.title), this);
                titleEdit->setPlaceholderText("Введите название карточки сюда...");
                titleEdit->setMinimumHeight(30);
                NeonInputFrame* titleBox = new NeonInputFrame(titleEdit, QColor("#38bdf8"), this);

                // Опциональное поле «Код / команда» — скрыто по умолчанию (g_cfg.showCodeField == false)
                QLabel* codeLbl = new QLabel("Отдельное поле «Код / команда»:", this);
                QPlainTextEdit* codeEdit = new QPlainTextEdit(QString::fromStdString(card.code), this);
                codeEdit->setPlaceholderText("Вставьте код или команду сюда...");
                codeEdit->setFont(QFont(MONO_FONT_FAMILY, 10));
                codeEdit->setMaximumHeight(85);
                NeonInputFrame* codeBox = new NeonInputFrame(codeEdit, QColor("#c084fc"), this);
                codeLbl->setVisible(g_cfg.showCodeField);
                codeBox->setVisible(g_cfg.showCodeField);

                // Чёткое разделение: Вкладка 1 — Описание на самой карточке; Вкладка 2 — Правая панель («Подробнее»)
                QTabWidget* contentTabs = new QTabWidget(this);

                RichTextEditor* descRichEditor = new RichTextEditor(this, 200, true);
                descRichEditor->setSerializedContent(card.description);

                RichTextEditor* extRichEditor = new RichTextEditor(this, 200, true);
                extRichEditor->setSerializedContent(card.extendedDesc);

                contentTabs->addTab(descRichEditor, "📇 1. Описание на карточке (основное поле и блоки ``` ```)");
                contentTabs->addTab(extRichEditor,  "📝 2. Правая панель («Подробнее» — секция ## Подробнее)");

                QCheckBox* pinCheck = new QCheckBox("Добавить в Избранное (★) — отображать карточку на главном экране по умолчанию", this);
                pinCheck->setChecked(card.pinned);

                QHBoxLayout* btnRow = new QHBoxLayout();
                NeonButton* attachBtn = new NeonButton("📎 Прикрепить файл в активную вкладку", QColor("#38bdf8"), this);
                NeonButton* cancelBtn = new NeonButton("Отмена", QColor("#94a3b8"), this);
                NeonButton* saveBtn   = new NeonButton("💾 Сохранить карточку", QColor("#4ade80"), this);
                attachBtn->setMinimumHeight(38);
                cancelBtn->setMinimumHeight(38);
                saveBtn->setMinimumHeight(38);

                btnRow->addWidget(attachBtn);
                btnRow->addStretch(1);
                btnRow->addWidget(cancelBtn);
                btnRow->addWidget(saveBtn);

                layout->addLayout(metaGrid);
                layout->addWidget(titleLbl);
                layout->addWidget(titleBox);
                layout->addWidget(codeLbl);
                layout->addWidget(codeBox);
                layout->addWidget(contentTabs, 1);
                layout->addWidget(pinCheck);
                layout->addLayout(btnRow);

                connect(attachBtn, &QPushButton::clicked, [contentTabs, descRichEditor, extRichEditor]() {
                    RichTextEditor* activeEd = (contentTabs->currentIndex() == 0) ? descRichEditor : extRichEditor;
                    if (activeEd && activeEd->rawEditor()) {
                        activeEd->rawEditor()->promptAndInsertAttachmentInStream();
                    }
                });

                connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);
                connect(saveBtn, &QPushButton::clicked, [=, &db]() mutable {
                    if (titleEdit->text().trimmed().isEmpty()) {
                        QMessageBox::warning(this, "Внимание", "Укажите заголовок карточки!");
                        return;
                    }
                    if (isNew || card.id == 0) {
                        card.id = static_cast<long long>(std::time(nullptr)) * 1000 + (std::rand() % 999);
                    }
                    card.pinned       = pinCheck->isChecked();
                    card.chapter      = chapterCombo->currentText().trimmed().toStdString();
                    card.section      = sectionCombo->currentText().trimmed().toStdString();
                    card.title        = titleEdit->text().trimmed().toStdString();
                    card.code         = g_cfg.showCodeField ? codeEdit->toPlainText().toStdString() : card.code;
                    card.description  = descRichEditor->getSerializedContent();
                    card.extendedDesc = extRichEditor->getSerializedContent();

                    db.saveCard(card, true);
                    accept();
                });
    }
};
