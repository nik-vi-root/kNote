#pragma once
#include "common.hpp"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QSyntaxHighlighter>
#include <QRegularExpression>

// ============================================================================
// 1. ПОДСВЕТКА СИНТАКСИСА PRISM.JS / OBSIDIAN
//    Поддерживает:
//    - Web (js, ts, html, css, scss, json, xml, svg)
//    - Backend (py, java, c, cpp, c++, csharp, c#, go, rust, php, ruby, swift, kotlin, lua, r, haskell, elixir)
//    - Shell / CLI (bash, sh, shell, console / shell-session с $ промптами, powershell, zsh, fish)
//    - Data / Config (sql, yaml, toml, ini, graphql, protobuf)
//    - Специальные (mermaid, dataview, math / latex, diff, git)
//    - Однострочные блоки ``` const ``` и многострочные блоки ```lang ... ```
// ============================================================================
class CodeSyntaxHighlighter : public QSyntaxHighlighter {
private:
    struct HighlightRule {
        QRegularExpression pattern;
        QTextCharFormat format;
    };

    QVector<HighlightRule> rules;
    QTextCharFormat fenceFormat;
    QTextCharFormat langBadgeFormat;
    QTextCharFormat codeBlockBaseFormat;
    QTextCharFormat inlineCodeFormat;
    QTextCharFormat diffAddFormat;
    QTextCharFormat diffDelFormat;
    QTextCharFormat consolePromptFormat;
    bool requireFences = true;

    static QString monoFontName() {
        #ifdef _WIN32
        return "Consolas";
        #else
        return "Monospace";
        #endif
    }

    void setupRules() {
        const QString mono = monoFontName();

        codeBlockBaseFormat.setFontFamily(mono);
        codeBlockBaseFormat.setForeground(QColor("#e2e8f0"));

        fenceFormat.setFontFamily(mono);
        fenceFormat.setFontWeight(QFont::Bold);
        fenceFormat.setForeground(QColor("#a855f7"));

        langBadgeFormat.setFontFamily(mono);
        langBadgeFormat.setFontWeight(QFont::Bold);
        langBadgeFormat.setForeground(QColor("#38bdf8"));

        inlineCodeFormat.setFontFamily(mono);
        inlineCodeFormat.setForeground(QColor("#fbbf24"));
        inlineCodeFormat.setBackground(QColor("#0f172a"));

        diffAddFormat.setFontFamily(mono);
        diffAddFormat.setForeground(QColor("#4ade80"));

        diffDelFormat.setFontFamily(mono);
        diffDelFormat.setForeground(QColor("#fb7185"));

        consolePromptFormat.setFontFamily(mono);
        consolePromptFormat.setFontWeight(QFont::Bold);
        consolePromptFormat.setForeground(QColor("#4ade80"));

        // 1. Ключевые слова (Web, Backend, Shell, SQL, Dataview, Mermaid)
        QTextCharFormat keywordFmt;
        keywordFmt.setFontFamily(mono);
        keywordFmt.setForeground(QColor("#38bdf8"));
        keywordFmt.setFontWeight(QFont::Bold);
        const QStringList keywords = {
            "alignas", "alignof", "and", "asm", "auto", "bool", "break", "case", "catch", "char",
            "class", "const", "constexpr", "continue", "default", "delete", "do", "double", "else",
            "enum", "explicit", "export", "extern", "false", "float", "for", "friend", "goto", "if",
            "inline", "int", "long", "mutable", "namespace", "new", "noexcept", "nullptr", "operator",
            "override", "private", "protected", "public", "register", "return", "short",
            "signed", "sizeof", "static", "struct", "switch", "template", "this", "throw", "true",
            "try", "typedef", "typename", "union", "unsigned", "using", "virtual", "void", "volatile",
            "while", "let", "var", "function", "async", "await", "import", "from", "extends", "super",
            "yield", "typeof", "instanceof", "in", "of", "echo", "foreach", "use", "def", "elif",
            "lambda", "pass", "raise", "with", "as", "fn", "mut", "pub", "impl", "trait", "match",
            "package", "func", "defer", "go", "chan", "select", "interface", "val", "fun", "when",
            "SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES", "UPDATE", "SET", "DELETE", "JOIN",
            "LEFT", "RIGHT", "INNER", "GROUP", "BY", "ORDER", "LIMIT", "CREATE", "TABLE", "ALTER",
            "DROP", "TASK", "LIST", "SORT", "ASC", "DESC", "FLATTEN", "GROUP BY",
            "graph", "flowchart", "sequenceDiagram", "classDiagram", "stateDiagram", "erDiagram",
            "gantt", "pie", "mindmap", "LR", "RL", "TB", "TD", "BT", "participant", "subgraph",
            "sudo", "docker", "systemctl", "apt", "git", "npm", "pnpm", "yarn", "cargo", "pip",
            "composer", "g\\+\\+", "gcc", "cmake", "make", "chmod", "chown", "ssh", "scp", "curl"
        };
        for (const QString& kw : keywords) {
            rules.append({QRegularExpression("\\b" + kw + "\\b"), keywordFmt});
        }

        // 2. Встроенные типы, стандартные объекты, HTML/XML теги, YAML/TOML/INI ключи и стрелки Mermaid
        QTextCharFormat typeFmt;
        typeFmt.setFontFamily(mono);
        typeFmt.setForeground(QColor("#c084fc"));
        typeFmt.setFontWeight(QFont::DemiBold);
        const QStringList types = {
            "std", "string", "vector", "map", "set", "pair", "shared_ptr", "unique_ptr", "size_t",
            "cout", "cin", "endl", "QWidget", "QString", "QApplication", "QMainWindow", "QPushButton",
            "QLabel", "document", "window", "console", "Array", "Object", "Promise", "JSON", "Math",
            "String", "Number", "Boolean", "Symbol", "BigInt", "Map", "Set", "WeakMap", "Date",
            "ref", "reactive", "computed", "watch", "onMounted", "useState", "useEffect",
            "body", "html", "div", "span", "font", "mark", "details", "summary", "iframe",
            "margin", "padding", "color", "background", "display", "flex", "grid", "font-family",
            "font-size", "border", "width", "height"
        };
        for (const QString& t : types) {
            rules.append({QRegularExpression("\\b" + t + "\\b"), typeFmt});
        }
        rules.append({QRegularExpression("^\\s*#[a-zA-Z_]+"), typeFmt});
        rules.append({QRegularExpression("</?[a-zA-Z0-9_-]+(\\s|>|/>)"), typeFmt});
        rules.append({QRegularExpression("^\\s*[a-zA-Z0-9_.-]+(?=\\s*[:=])"), typeFmt});
        rules.append({QRegularExpression("(-->|---|==>|-.->|==>|<-->)"), typeFmt});

        // 3. Вызовы функций и методов: имя(
        QTextCharFormat funcFmt;
        funcFmt.setFontFamily(mono);
        funcFmt.setForeground(QColor("#60a5fa"));
        rules.append({QRegularExpression("\\b[A-Za-z0-9_]+(?=\\s*\\()"), funcFmt});

        // 4. Числа и единицы измерения (px, rem, em, %, s, ms)
        QTextCharFormat numFmt;
        numFmt.setFontFamily(mono);
        numFmt.setForeground(QColor("#fb923c"));
        rules.append({QRegularExpression("\\b(0x[0-9a-fA-F]+|\\d+(\\.\\d+)?(px|rem|em|vh|vw|pt|s|ms|%)?)\\b"), numFmt});

        // 5. Строки ("...", '...', `...`)
        QTextCharFormat strFmt;
        strFmt.setFontFamily(mono);
        strFmt.setForeground(QColor("#4ade80"));
        rules.append({QRegularExpression("\"([^\"\\\\]|\\\\.)*\""), strFmt});
        rules.append({QRegularExpression("'([^'\\\\]|\\\\.)*'"), strFmt});

        // 6. Комментарии (//..., /*...*/, # комментарий в Python/Bash/YAML)
        QTextCharFormat commentFmt;
        commentFmt.setFontFamily(mono);
        commentFmt.setForeground(QColor("#94a3b8"));
        commentFmt.setFontItalic(true);
        rules.append({QRegularExpression("//[^\n]*"), commentFmt});
        rules.append({QRegularExpression("/\\*.*\\*/"), commentFmt});
        rules.append({QRegularExpression("(?<=\\s)#\\s+[^\n]*"), commentFmt});
    }

    void applyCodeRulesToRange(const QString& text, int startOffset, int length) {
        if (length <= 0) return;
        QString sub = text.mid(startOffset, length);

        setFormat(startOffset, length, codeBlockBaseFormat);

        // Поддержка console / shell-session ($ npm install) и diff (+ / -)
        QString trimmedSub = sub.trimmed();
        if (trimmedSub.startsWith("$ ") || trimmedSub.startsWith("PS ") || trimmedSub.startsWith("> ")) {
            int promptLen = sub.indexOf(trimmedSub[0]) + 2;
            setFormat(startOffset, promptLen, consolePromptFormat);
        } else if (sub.startsWith("+") && !sub.startsWith("+++")) {
            setFormat(startOffset, length, diffAddFormat);
            return;
        } else if (sub.startsWith("-") && !sub.startsWith("---")) {
            setFormat(startOffset, length, diffDelFormat);
            return;
        }

        for (const HighlightRule& rule : rules) {
            QRegularExpressionMatchIterator it = rule.pattern.globalMatch(sub);
            while (it.hasNext()) {
                QRegularExpressionMatch m = it.next();
                setFormat(startOffset + m.capturedStart(), m.capturedLength(), rule.format);
            }
        }
    }

public:
    CodeSyntaxHighlighter(QTextDocument* parentDoc, bool fencedMode = true)
    : QSyntaxHighlighter(parentDoc), requireFences(fencedMode)
    {
        setupRules();
    }

protected:
    void highlightBlock(const QString& text) override {
        if (!requireFences) {
            applyCodeRulesToRange(text, 0, text.length());
            return;
        }

        int prevState = previousBlockState();
        bool inCodeBlock = (prevState == 1);

        QString trimmed = text.trimmed();

        if (trimmed.startsWith("```")) {
            setFormat(0, text.length(), fenceFormat);
            int firstFence = text.indexOf("```");

            // Однострочный кодовый блок вида ``` const ``` или ```js const x = 1; ```
            if (trimmed.length() > 6 && trimmed.endsWith("```")) {
                int lastFence = text.lastIndexOf("```");
                if (lastFence > firstFence + 3) {
                    applyCodeRulesToRange(text, firstFence + 3, lastFence - (firstFence + 3));
                }
                setCurrentBlockState(inCodeBlock ? 1 : 0);
            } else {
                // Подсветка идентификатора языка после открывающих ``` (например ```javascript, ```cpp, ```console)
                if (!inCodeBlock && firstFence >= 0 && text.length() > firstFence + 3) {
                    setFormat(firstFence + 3, text.length() - (firstFence + 3), langBadgeFormat);
                }
                setCurrentBlockState(inCodeBlock ? 0 : 1);
            }
            return;
        }

        if (inCodeBlock) {
            applyCodeRulesToRange(text, 0, text.length());
            setCurrentBlockState(1);
            return;
        }

        setCurrentBlockState(0);

        // Однострочные ``` ... ``` внутри обычной строки
        static QRegularExpression inlineTripleRe("```([^`]+)```");
        QRegularExpressionMatchIterator itTriple = inlineTripleRe.globalMatch(text);
        while (itTriple.hasNext()) {
            QRegularExpressionMatch m = itTriple.next();
            setFormat(m.capturedStart(), m.capturedLength(), fenceFormat);
            applyCodeRulesToRange(text, m.capturedStart() + 3, m.capturedLength() - 6);
        }

        // Инлайн-код `...`
        static QRegularExpression inlineSingleRe("(?<!`)`([^`]+)`(?!`)");
        QRegularExpressionMatchIterator itSingle = inlineSingleRe.globalMatch(text);
        while (itSingle.hasNext()) {
            QRegularExpressionMatch m = itSingle.next();
            setFormat(m.capturedStart(), m.capturedLength(), inlineCodeFormat);
        }
    }
};

class SyntaxAutoAttacher : public QObject {
public:
    explicit SyntaxAutoAttacher(QObject* parent = nullptr) : QObject(parent) {}

    static void attachToEditor(QWidget* w) {
        if (!w || w->property("_hasSyntaxHighlighter").toBool()) return;

        if (auto* te = qobject_cast<QTextEdit*>(w)) {
            w->setProperty("_hasSyntaxHighlighter", true);
            bool prevBlocked = te->blockSignals(true);
            new CodeSyntaxHighlighter(te->document(), true);
            te->blockSignals(prevBlocked);
        } else if (auto* pte = qobject_cast<QPlainTextEdit*>(w)) {
            w->setProperty("_hasSyntaxHighlighter", true);
            bool prevBlocked = pte->blockSignals(true);
            new CodeSyntaxHighlighter(pte->document(), false);
            pte->blockSignals(prevBlocked);
        }
    }

protected:
    bool eventFilter(QObject* watched, QEvent* event) override {
        if (event->type() == QEvent::Show || event->type() == QEvent::FocusIn) {
            if (QWidget* w = qobject_cast<QWidget*>(watched)) {
                if (qobject_cast<QTextEdit*>(w) || qobject_cast<QPlainTextEdit*>(w)) {
                    attachToEditor(w);
                }
            }
        } else if (event->type() == QEvent::KeyPress) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Tab && ke->modifiers() == Qt::NoModifier) {
                QPlainTextEdit* pte = qobject_cast<QPlainTextEdit*>(watched);
                if (!pte && watched && watched->parent()) {
                    pte = qobject_cast<QPlainTextEdit*>(watched->parent());
                }
                if (pte && !pte->isReadOnly()) {
                    pte->insertPlainText("  ");
                    return true;
                }
            }
        }
        return QObject::eventFilter(watched, event);
    }
};

inline void installSyntaxAutoAttacher() {
    if (!qApp) return;
    qApp->installEventFilter(new SyntaxAutoAttacher(qApp));
}
Q_COREAPP_STARTUP_FUNCTION(installSyntaxAutoAttacher)

// ============================================================================
// 2. ПАНЕЛЬ ИНСТРУМЕНТОВ ФОРМАТИРОВАНИЯ ТЕКСТА (Включая кнопку «📎 Файл»)
// ============================================================================
class FormattingToolbarPanel : public QFrame {
private:
    QTextEdit* explicitTarget = nullptr;

    QTextEdit* targetEditor() const {
        return explicitTarget ? explicitTarget : g_activeRichEditor;
    }

public:
    FormattingToolbarPanel(QTextEdit* fixedTarget = nullptr, QWidget* parent = nullptr, std::function<void()> onClose = nullptr)
    : QFrame(parent), explicitTarget(fixedTarget)
    {
        if (explicitTarget) {
            SyntaxAutoAttacher::attachToEditor(explicitTarget);
        }

        QVBoxLayout* root = new QVBoxLayout(this);
        root->setContentsMargins(12, 10, 12, 10);
        root->setSpacing(8);

        QHBoxLayout* row1 = new QHBoxLayout();
        row1->setSpacing(6);

        ColorPickerButton* strokeBtn = new ColorPickerButton(
            PickerKind::StrokeBg,
            DEFAULT_BG_COLORS[0].color,
            [this](QColor bgColor) {
                QTextEdit* ed = targetEditor();
                if (!ed) return;
                QTextCursor cursor = ed->textCursor();
                QTextCharFormat current = cursor.charFormat();
                QTextCharFormat fmt;
                fmt.setBackground(bgColor);
                QColor fg = current.foreground().color();
                if (!current.hasProperty(QTextFormat::ForegroundBrush) ||
                    std::abs(fg.lightness() - bgColor.lightness()) < 75) {
                    fmt.setForeground(getReadableTextColorForBg(bgColor));
                    }
                    cursor.mergeCharFormat(fmt);
                ed->mergeCurrentCharFormat(fmt);
                ed->setFocus();
            },
            this
        );

        ColorPickerButton* textBtn = new ColorPickerButton(
            PickerKind::TextFg,
            DEFAULT_FG_COLORS[0].color,
            [this](QColor fgColor) {
                QTextEdit* ed = targetEditor();
                if (!ed) return;
                QTextCharFormat fmt;
                fmt.setForeground(fgColor);
                QTextCursor cursor = ed->textCursor();
                cursor.mergeCharFormat(fmt);
                ed->mergeCurrentCharFormat(fmt);
                ed->setFocus();
            },
            this
        );

        ColorPickerButton* underlineBtn = new ColorPickerButton(
            PickerKind::Underline,
            DEFAULT_FG_COLORS[1].color,
            [this](QColor ulColor) {
                QTextEdit* ed = targetEditor();
                if (!ed) return;
                QTextCharFormat fmt;
                fmt.setFontUnderline(true);
                fmt.setUnderlineStyle(QTextCharFormat::SingleUnderline);
                fmt.setUnderlineColor(ulColor);
                QTextCursor cursor = ed->textCursor();
                cursor.mergeCharFormat(fmt);
                ed->mergeCurrentCharFormat(fmt);
                ed->setFocus();
            },
            this
        );

        QSpinBox* fontSizeSpin = new QSpinBox(this);
        fontSizeSpin->setRange(8, 36);
        fontSizeSpin->setValue(11);
        fontSizeSpin->setSuffix(" pt");
        fontSizeSpin->setFixedHeight(32);

        QComboBox* fontWeightCombo = new QComboBox(this);
        fontWeightCombo->setFixedHeight(32);
        fontWeightCombo->addItem("Тонкий", static_cast<int>(QFont::Light));
        fontWeightCombo->addItem("Обычный", static_cast<int>(QFont::Normal));
        fontWeightCombo->addItem("Средний", static_cast<int>(QFont::Medium));
        fontWeightCombo->addItem("Полужирный", static_cast<int>(QFont::DemiBold));
        fontWeightCombo->addItem("Жирный", static_cast<int>(QFont::Bold));
        fontWeightCombo->addItem("Сверхжирный", static_cast<int>(QFont::Black));
        fontWeightCombo->setCurrentIndex(1);

        row1->addWidget(strokeBtn);
        row1->addWidget(textBtn);
        row1->addWidget(underlineBtn);
        row1->addWidget(fontSizeSpin);
        row1->addWidget(fontWeightCombo);

        if (onClose) {
            NeonButton* closeMiniBtn = new NeonButton("×", QColor("#fb7185"), this);
            closeMiniBtn->setFixedSize(30, 30);
            closeMiniBtn->setFocusPolicy(Qt::NoFocus);
            closeMiniBtn->setToolTip("Закрыть док-панель");
            connect(closeMiniBtn, &QPushButton::clicked, [onClose]() { onClose(); });
            row1->addWidget(closeMiniBtn);
        } else {
            row1->addStretch(1);
        }

        QHBoxLayout* row2 = new QHBoxLayout();
        row2->setSpacing(6);

        NeonButton* alignLeftBtn   = new NeonButton("⬅", QColor("#38bdf8"), this);
        NeonButton* alignCenterBtn = new NeonButton("↔", QColor("#2dd4bf"), this);
        NeonButton* alignRightBtn  = new NeonButton("➡", QColor("#f472b6"), this);
        alignLeftBtn->setFixedSize(36, 30);
        alignCenterBtn->setFixedSize(36, 30);
        alignRightBtn->setFixedSize(36, 30);
        alignLeftBtn->setFocusPolicy(Qt::NoFocus);
        alignCenterBtn->setFocusPolicy(Qt::NoFocus);
        alignRightBtn->setFocusPolicy(Qt::NoFocus);

        NeonButton* insertCodeBtn = new NeonButton("</> Код", QColor("#4ade80"), this);
        insertCodeBtn->setFixedHeight(30);
        insertCodeBtn->setFocusPolicy(Qt::NoFocus);
        insertCodeBtn->setToolTip("Вставить блок кода ``` ... ```");

        NeonButton* insertImgBtn  = new NeonButton("🖼 Картинка", QColor("#a855f7"), this);
        NeonButton* attachFileBtn = new NeonButton("📎 Файл", QColor("#38bdf8"), this);
        NeonButton* clearFmtBtn   = new NeonButton("✨ Очистить", QColor("#fb7185"), this);
        insertImgBtn->setFixedHeight(30);
        attachFileBtn->setFixedHeight(30);
        clearFmtBtn->setFixedHeight(30);
        insertImgBtn->setFocusPolicy(Qt::NoFocus);
        attachFileBtn->setFocusPolicy(Qt::NoFocus);
        clearFmtBtn->setFocusPolicy(Qt::NoFocus);
        attachFileBtn->setToolTip("Прикрепить файл прямо в поток текста как [[имя_файла]]");

        row2->addWidget(alignLeftBtn);
        row2->addWidget(alignCenterBtn);
        row2->addWidget(alignRightBtn);
        row2->addWidget(insertCodeBtn);
        row2->addWidget(insertImgBtn);
        row2->addWidget(attachFileBtn);
        row2->addWidget(clearFmtBtn);
        row2->addStretch(1);

        root->addLayout(row1);
        root->addLayout(row2);

        connect(fontSizeSpin, QOverload<int>::of(&QSpinBox::valueChanged), [this](int sz) {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            QTextCharFormat fmt;
            fmt.setFontPointSize(sz);
            QTextCursor cursor = ed->textCursor();
            cursor.mergeCharFormat(fmt);
            ed->mergeCurrentCharFormat(fmt);
            ed->setFocus();
        });

        connect(fontWeightCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), [this, fontWeightCombo](int idx) {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            int weight = fontWeightCombo->itemData(idx).toInt();
            QTextCharFormat fmt;
            fmt.setFontWeight(weight);
            QTextCursor cursor = ed->textCursor();
            cursor.mergeCharFormat(fmt);
            ed->mergeCurrentCharFormat(fmt);
            ed->setFocus();
        });

        auto applyAlign = [this](Qt::Alignment a) {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            QTextCursor cursor = ed->textCursor();
            QTextBlockFormat bf = cursor.blockFormat();
            bf.setAlignment(a);
            cursor.mergeBlockFormat(bf);
            ed->setFocus();
        };

        connect(alignLeftBtn,   &QPushButton::clicked, [applyAlign]() { applyAlign(Qt::AlignLeft); });
        connect(alignCenterBtn, &QPushButton::clicked, [applyAlign]() { applyAlign(Qt::AlignHCenter); });
        connect(alignRightBtn,  &QPushButton::clicked, [applyAlign]() { applyAlign(Qt::AlignRight); });

        connect(insertCodeBtn, &QPushButton::clicked, [this]() {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            SyntaxAutoAttacher::attachToEditor(ed);

            QTextCursor cursor = ed->textCursor();
            QTextCharFormat cleanCodeFmt;
            cleanCodeFmt.setFontFamily(MONO_FONT_FAMILY);
            cleanCodeFmt.setFontPointSize(11);
            cleanCodeFmt.setFontWeight(QFont::Normal);
            cleanCodeFmt.setFontUnderline(false);
            cleanCodeFmt.setForeground(QColor(240, 246, 255));
            cleanCodeFmt.clearBackground();

            cursor.beginEditBlock();
            if (cursor.hasSelection()) {
                QString selected = cursor.selectedText();
                selected.replace(QChar::ParagraphSeparator, '\n');
                selected.replace(QChar::LineSeparator, '\n');
                cursor.insertText("```\n" + selected + "\n```", cleanCodeFmt);
            } else {
                if (!cursor.atBlockStart()) {
                    cursor.insertText("\n", cleanCodeFmt);
                }
                cursor.insertText("```\n\n```", cleanCodeFmt);
                cursor.movePosition(QTextCursor::Left, QTextCursor::MoveAnchor, 4);
                ed->setTextCursor(cursor);
            }
            cursor.endEditBlock();
            if (ed->viewport()) ed->viewport()->update();
            ed->setFocus();
        });

        connect(insertImgBtn, &QPushButton::clicked, [this]() {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            QString fileName = QFileDialog::getOpenFileName(
                this, "Выберите изображение", "", "Изображения (*.png *.jpg *.jpeg *.webp *.bmp *.gif *.svg)"
            );
            if (!fileName.isEmpty()) {
                QString savedPath = ImageStorage::copyImageFileToStorage(fileName);
                if (!savedPath.isEmpty()) {
                    QTextCursor cursor = ed->textCursor();
                    ImageStorage::insertImageAtCursor(cursor, savedPath, 340);
                    ed->setFocus();
                }
            }
        });

        connect(attachFileBtn, &QPushButton::clicked, [this]() {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            QStringList files = QFileDialog::getOpenFileNames(
                this, "Прикрепить файл в поток текста (как в Obsidian)", "", "Все файлы (*.*)"
            );
            if (!files.isEmpty()) {
                QTextCursor cursor = ed->textCursor();
                for (const QString& f : files) {
                    std::string fn = AttachmentStorage::attachFileToVault(f);
                    if (!fn.empty()) {
                        MarkdownCodec::insertFileEmbedAtCursor(cursor, QString::fromStdString(fn), false);
                    }
                }
                ed->setFocus();
            }
        });

        connect(clearFmtBtn, &QPushButton::clicked, [this]() {
            QTextEdit* ed = targetEditor();
            if (!ed) return;
            QTextCursor cursor = ed->textCursor();
            if (!cursor.hasSelection()) cursor.select(QTextCursor::Document);
            QTextCharFormat cleanFmt;
            cleanFmt.setFontPointSize(11);
            cleanFmt.setFontWeight(QFont::Normal);
            cleanFmt.setFontItalic(false);
            cleanFmt.setFontStrikeOut(false);
            cleanFmt.setFontUnderline(false);
            cleanFmt.setForeground(QColor(240, 246, 255));
            cleanFmt.clearBackground();
            cursor.setCharFormat(cleanFmt);
            ed->setFocus();
        });
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);

        QRectF r = QRectF(rect()).adjusted(1.5, 1.5, -1.5, -1.5);
        p.setBrush(g_cfg.bgTrackerCard);
        QColor bc = g_cfg.isMaleTheme ? QColor(56, 189, 248, 220) : QColor(244, 114, 182, 220);
        p.setPen(QPen(bc, 1.8));
        p.drawRoundedRect(r, 12, 12);
    }
};

// ============================================================================
// 3. ПЛАВАЮЩАЯ ДОК-ПАНЕЛЬ У КУРСОРА МЫШИ
// ============================================================================
class FloatingFormatMiniBar : public QFrame {
private:
    bool dragging = false;
    QPoint dragOffset;

public:
    FloatingFormatMiniBar(QWidget* parentWindow)
    : QFrame(parentWindow)
    {
        setFocusPolicy(Qt::NoFocus);
        setCursor(Qt::SizeAllCursor);

        QVBoxLayout* l = new QVBoxLayout(this);
        l->setContentsMargins(6, 6, 6, 6);

        FormattingToolbarPanel* miniPanel = new FormattingToolbarPanel(
            nullptr,
            this,
            [this]() { hide(); }
        );
        miniPanel->setCursor(Qt::ArrowCursor);

        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(miniPanel);
        shadow->setBlurRadius(26);
        shadow->setOffset(0, 8);
        shadow->setColor(QColor(0, 0, 0, 200));
        miniPanel->setGraphicsEffect(shadow);

        l->addWidget(miniPanel);
        adjustSize();
    }

    void toggleAtCursor() {
        if (isVisible()) {
            hide();
            return;
        }
        if (!parentWidget()) return;

        adjustSize();

        QPoint globalCursor = QCursor::pos();
        QPoint localCursor  = parentWidget()->mapFromGlobal(globalCursor);

        int targetX = localCursor.x() + 14;
        int targetY = localCursor.y() - height() / 2;

        int maxX = std::max(8, parentWidget()->width()  - width()  - 8);
        int maxY = std::max(8, parentWidget()->height() - height() - 8);
        targetX = std::clamp(targetX, 8, maxX);
        targetY = std::clamp(targetY, 8, maxY);

        move(targetX, targetY);
        show();
        raise();
    }

protected:
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            dragging = true;
            dragOffset = e->globalPos() - pos();
            raise();
        }
        QFrame::mousePressEvent(e);
    }

    void mouseMoveEvent(QMouseEvent* e) override {
        if (dragging && parentWidget()) {
            QPoint nextPos = e->globalPos() - dragOffset;
            int maxX = std::max(4, parentWidget()->width()  - width()  - 4);
            int maxY = std::max(4, parentWidget()->height() - height() - 4);
            nextPos.setX(std::clamp(nextPos.x(), 4, maxX));
            nextPos.setY(std::clamp(nextPos.y(), 4, maxY));
            move(nextPos);
        }
        QFrame::mouseMoveEvent(e);
    }

    void mouseReleaseEvent(QMouseEvent* e) override {
        dragging = false;
        QFrame::mouseReleaseEvent(e);
    }
};
