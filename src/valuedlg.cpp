#include "valuedlg.h"
#include "ui_valuedlg.h"

#include "logtab.h"
#include "themeutils.h"
#include "tokenizer.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFile>
#include <QFontDatabase>
#include <QHttpMultiPart>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>
#include <QString>
#include <QTextDocument>
#include <QTextStream>
#include <QWebEngineView>

QByteArray ValueDlg::sm_savedGeometry { QByteArray() };
qreal ValueDlg::sm_savedFontPointSize { 0 };
bool ValueDlg::sm_savedWrapText { true };
QJsonUtils::Notation ValueDlg::sm_notation { QJsonUtils::Notation::YAML };

ValueDlg::ValueDlg(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ValueDlg)
{
    ui->setupUi(this);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    restoreGeometry(sm_savedGeometry);

    // Set a monospaced font.
    QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    fixedFont.setPointSizeF(sm_savedFontPointSize);
    ui->textEdit->setFont(fixedFont);

    auto prevKey = QKeySequence(Qt::CTRL | Qt::Key_Up);
    ui->prevButton->setShortcut(prevKey);
    ui->prevButton->setToolTip(QString("Go to previous event (%1)").arg(prevKey.toString(QKeySequence::NativeText)));
    auto nextKey = QKeySequence(Qt::CTRL | Qt::Key_Down);
    ui->nextButton->setShortcut(nextKey);
    ui->nextButton->setToolTip(QString("Go to next event (%1)").arg(nextKey.toString(QKeySequence::NativeText)));

    QShortcut *shortcutPlus = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Plus), this);
    connect(shortcutPlus, &QShortcut::activated, this, &ValueDlg::on_zoomIn);
    QShortcut *shortcutEqual = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Equal), this);
    connect(shortcutEqual, &QShortcut::activated, this, &ValueDlg::on_zoomIn);
    QShortcut *shortcutMinus = new QShortcut(QKeySequence(Qt::CTRL | Qt::Key_Minus), this);
    connect(shortcutMinus, &QShortcut::activated, this, &ValueDlg::on_zoomOut);

    m_id = QString("");
    m_key = QString("");
    m_queryPlan = QString("");
    Options& options = Options::GetInstance();
    m_visualizationServiceEnable = options.getVisualizationServiceEnable();
    m_visualizationServiceURL = options.getVisualizationServiceURL();

    ui->prevButton->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/value-previous.png")));
    ui->nextButton->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/value-next.png")));
    ui->wrapTextCheck->setChecked(sm_savedWrapText);
    SetWrapping(sm_savedWrapText);

    {
        const QSignalBlocker blocker(ui->notationComboBox);
        ui->notationComboBox->addItems(QJsonUtils::GetNotationNames());
        ui->notationComboBox->setCurrentText(QJsonUtils::GetNameForNotation(sm_notation));
    }

    QFile sqlsyntaxcss(":/sqlsyntax.css");
    sqlsyntaxcss.open(QIODevice::ReadOnly);
    QTextStream textStream(&sqlsyntaxcss);
    QString styleSheet = textStream.readAll();
    sqlsyntaxcss.close();

    QTextDocument * textDoc = new QTextDocument(ui->textEdit);
    textDoc->setDefaultStyleSheet(styleSheet);
    textDoc->setDefaultFont(fixedFont);
    ui->textEdit->setDocument(textDoc);
}

ValueDlg::~ValueDlg()
{
    delete ui;
}

void ValueDlg::on_zoomIn()
{
    ui->textEdit->zoomIn(1);
}

void ValueDlg::on_zoomOut()
{
    ui->textEdit->zoomOut(1);
}

static QString MakeHTMLSafe(QString input)
{
    auto output = input.toHtmlEscaped();
    // Use HTML safe spacing
    output.replace(" ", "&nbsp;");
    output.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
    output.replace("\n", "<br>");

    return output;
}

static QString HighlightSyntax(QString in)
{
    in = in.replace("; ", "\n");
    Tokenizer sqlLexer;
    sqlLexer.SetSQL();
    QList<Tokenizer::Token> tokens = sqlLexer.Tokenize(in);
    if (tokens.size() == 0)
        return in;

    QString out;

    QHash<Tokenizer::TokenType, QString> TokenTypeToCSSClass = {
        {Tokenizer::TokenType::Keyword, "k"},
        {Tokenizer::TokenType::String, "s"},
        {Tokenizer::TokenType::Tag, "t"},
        {Tokenizer::TokenType::Whitespace, "w"},
        {Tokenizer::TokenType::Identifier, "i"},
        {Tokenizer::TokenType::Symbol, "sy"},
        {Tokenizer::TokenType::OpenParan, "op"},
        {Tokenizer::TokenType::CloseParan, "cp"},
        {Tokenizer::TokenType::Operator, "o"},
        {Tokenizer::TokenType::NoMatch, "n"},
        {Tokenizer::TokenType::Integer, "in"},
        {Tokenizer::TokenType::Float, "f"}
    };

    // Build the html, grouping concurrent same token items in the same span.
    Tokenizer::TokenType prevType = tokens[0].type;
    out += "<span class=\"" + TokenTypeToCSSClass[prevType] + "\">";
    for (Tokenizer::Token token : tokens)
    {
        QString word = in.mid(token.start, token.length);
        word = MakeHTMLSafe(word);
        if(token.type != prevType)
        {
            out += "</span><span class=\"" + TokenTypeToCSSClass[token.type] + "\">";
            prevType = token.type;
        }
        out += word;
    }
    out += "</span>";
    return out;
}

void ValueDlg::SetContent(QString id, QString key, QJsonValue value)
{
    m_id = id;
    m_key = key;
    m_value = value;

    setWindowTitle(QString("ID: %1 - Key: %2").arg(m_id, m_key));

    UpdateValueBox();

    // Recognize query plans
    m_queryPlan = "";
    if (value.isObject()) {
        if (m_key.startsWith("logical-query") ||
            m_key.startsWith("aql-table") ||
            m_key == "federate-query" ||
            m_key == "remote-query-planning" ||
            m_key == "begin-query" ||
            m_key == "begin-protocol.query" ||
            m_key == "end-query" ||
            m_key == "end-protocol.query")
        {
            // Assume that queries starting with < have query function trees or logical-query.
            // They normally start with "<?xml ...>", "<sqlproxy>" or "<query-function ...>"
            auto queryText = value.toObject()["query"].toString();
            if (queryText.startsWith("<"))
            {
                m_queryPlan = queryText;
            }
        }
        else if (m_key.startsWith("query-plan") || m_key == "optimizer-step")
        {
            auto plan = value.toObject()["plan"];
            if (plan.isObject()) {
                QJsonDocument doc(plan.toObject());
                m_queryPlan = doc.toJson(QJsonDocument::Compact);
            }
        }
    }
    // Setup UI for query plan
    if (!m_queryPlan.isEmpty())
    {
        ui->visualizeButton->setEnabled(true);
        ui->visualizeLabel->setText("");
    }
    else
    {
        ui->visualizeButton->setEnabled(false);
        ui->visualizeLabel->setText("Nothing to visualize");
    }

    ui->notationComboBox->setEnabled(QJsonUtils::IsStructured(value));
    ui->notationComboBox->repaint();
}

void ValueDlg::UpdateValueBox() {
    QString value = QJsonUtils::Format(m_value, sm_notation);

    int syntaxHighlightLimit = Options::GetInstance().getSyntaxHighlightLimit();
    bool syntaxHighlight = (!m_key.isEmpty() &&
                            QJsonUtils::IsStructured(m_value) &&
                            syntaxHighlightLimit &&
                            value.size() <= syntaxHighlightLimit);
    if (syntaxHighlight)
    {
        QString htmlText(QString("<body>") + HighlightSyntax(value) + QString("</body>"));
        ui->textEdit->setHtml(htmlText);
    }
    else
    {
        // Toggling between "setHtml" and "setPlainText" still winds up formatting
        // the "plain text" based on the previous HTML contents. The simplest way
        // to keep it "plain" appears to be to just keep it HTML and omit the styling.
        // (alternatively, we could explicitly use setTextColor to make it black..
        //  but that might not play well with themes?)
        QString htmlText(QString("<body><span>%1</span></body>").arg(MakeHTMLSafe(value)));
        ui->textEdit->setHtml(htmlText);
    }
    ui->textEdit->moveCursor(QTextCursor::Start);
    ui->textEdit->ensureCursorVisible();
    ui->textEdit->repaint();
}

void ValueDlg::on_nextButton_clicked()
{
    emit next();
}

void ValueDlg::on_prevButton_clicked()
{
    emit prev();
}

void ValueDlg::on_wrapTextCheck_clicked()
{
    SetWrapping(ui->wrapTextCheck->isChecked());
}

void ValueDlg::on_notationComboBox_currentTextChanged(const QString& newValue)
{
    sm_notation = QJsonUtils::GetNotationFromName(newValue);
    UpdateValueBox();
}

void ValueDlg::SetWrapping(const bool wrapText)
{
    sm_savedWrapText = wrapText;
    if (sm_savedWrapText)
    {
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    }
    else
    {
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
    }
}

QUrl ValueDlg::GetUploadURL()
{
    return QUrl(m_visualizationServiceURL);
}

QUrl ValueDlg::GetVisualizeURL(QNetworkReply * const reply)
{
    // The URL to visualize the query should be in the response.
    //
    // Sample response:
    //http://localhost:3000/d3/query-graphs.html?upload=y&file=31480681.xml
    auto response = QString(reply->readAll());
    qDebug() << "Response from visualization service:\n" << response;
    return QUrl(response);
}

void ValueDlg::UploadQuery()
{
    QHttpMultiPart *multipart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);
    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"queryfile\"; filename=\"yeah.xml\""));
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/xml"));

    filePart.setBody(m_queryPlan.toUtf8());

    multipart->append(filePart);

    QNetworkRequest request(GetUploadURL());
    QNetworkAccessManager *networkManager = new QNetworkAccessManager(this);
    QNetworkReply *reply = networkManager->post(request, multipart);
    // Set the parent of the multipart to the reply, so that we delete the multiPart with the reply
    multipart->setParent(reply);

    connect(networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(on_uploadFinished(QNetworkReply*)));
}

void ValueDlg::on_visualizeButton_clicked()
{
    if (m_visualizationServiceEnable)
    {
        UploadQuery();
    }
    else
    {
        VisualizeQuery();
    }
}

void ValueDlg::on_uploadFinished(QNetworkReply* reply)
{
    if (reply->error() == QNetworkReply::NetworkError::NoError)
    {
        ui->visualizeLabel->setText("Visualization uploaded, check your browser...");
        QDesktopServices::openUrl(GetVisualizeURL(reply));
    }
    else
    {
        ui->visualizeLabel->setText(QString("Failed to upload to URL '%1'").arg(reply->url().toString()));
        qDebug() << "Failed to upload query";
    }
    ui->visualizeButton->setEnabled(false);
}

void ValueDlg::VisualizeQuery()
{
    // Conditionally enable QtWebEngine debugging via Chromium-based browser at http://localhost:9000
#ifdef QT_DEBUG
    qputenv("QTWEBENGINE_REMOTE_DEBUGGING", "9000");
#endif

    m_view = new QWebEngineView(this);
    m_view->setAttribute(Qt::WA_DeleteOnClose);

    // Set Qt::Dialog rather than Qt::Window to disable zoom and avoid a Qt bug with unresponsive zoomed views on OSX
    m_view->setWindowFlags(Qt::Dialog | Qt::WindowMaximizeButtonHint | Qt::WindowCloseButtonHint);
    m_view->setWindowTitle(QString("Query Visualization - ID: %1 - Key: %2").arg(m_id, m_key));

    QScreen* screen = this->windowHandle()->screen();
    QRect screenRect = screen->availableGeometry();
    int width = screenRect.width() - 400;
    int height = screenRect.height() - 200;
    int x = screenRect.left() + 200;
    int y = screenRect.top() + 80;
    m_view->resize(width, height);
    m_view->move(x, y);

    // Add keyboard shortcut for reloading the view
    QShortcut *shortcut = new QShortcut(QKeySequence("Ctrl+r"), m_view);
    QObject::connect(shortcut, &QShortcut::activated, m_view, &QWebEngineView::reload);

    // Reload the view one time to work around a Qt bug causing left justified alignment
    m_reload = true;
    connect(m_view, &QWebEngineView::loadFinished, this, &ValueDlg::on_loadFinished);
    m_view->load(QUrl("qrc:///query-graphs/query-graphs.tlv.html?inline=" + QUrl::toPercentEncoding(m_queryPlan)));
}

void ValueDlg::on_loadFinished(bool loaded)
{
    if (!loaded)
    {
        ui->visualizeLabel->setText(QString("Failed to load query visualization"));
        qDebug() << "Failed to load query visualization";
    }
    if (m_reload)
    {
        m_reload = false;
        m_view->reload();
    }
    m_view->show();
}

void ValueDlg::reject()
{
    sm_savedGeometry = saveGeometry();
    sm_savedFontPointSize = ui->textEdit->document()->defaultFont().pointSize();
    QDialog::reject();
}

// Persist the dialog state into QSettings.
//
// WriteSettings and ReadSettings should only be called once during app start and close,
// so the dialog state between multiple instances will not interfere each other.
//
void ValueDlg::WriteSettings(QSettings& settings)
{
    settings.beginGroup("ValueDialog");
    settings.setValue("geometry", sm_savedGeometry);
    settings.setValue("fontPointSize", sm_savedFontPointSize);
    settings.setValue("wrapText", sm_savedWrapText);
    settings.setValue("notation", QJsonUtils::GetNameForNotation(sm_notation));
    settings.endGroup();
}

void ValueDlg::ReadSettings(QSettings& settings)
{
    settings.beginGroup("ValueDialog");
    sm_savedGeometry = settings.value("geometry").toByteArray();
    sm_savedFontPointSize = settings.value("fontPointSize").toReal();
    sm_savedWrapText = settings.value("wrapText").toBool();
    sm_notation = QJsonUtils::GetNotationFromName(settings.value("notation").toString());
    settings.endGroup();
}
