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

    auto prevKey = QKeySequence(Qt::CTRL + Qt::Key_Up);
    ui->prevButton->setShortcut(prevKey);
    ui->prevButton->setToolTip(QString("Go to previous event (%1)").arg(prevKey.toString(QKeySequence::NativeText)));
    auto nextKey = QKeySequence(Qt::CTRL + Qt::Key_Down);
    ui->nextButton->setShortcut(nextKey);
    ui->nextButton->setToolTip(QString("Go to next event (%1)").arg(nextKey.toString(QKeySequence::NativeText)));

    QShortcut *shortcutPlus = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Plus), this);
    connect(shortcutPlus, &QShortcut::activated, this, &ValueDlg::on_zoomIn);
    QShortcut *shortcutEqual = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Equal), this);
    connect(shortcutEqual, &QShortcut::activated, this, &ValueDlg::on_zoomIn);
    QShortcut *shortcutMinus = new QShortcut(QKeySequence(Qt::CTRL + Qt::Key_Minus), this);
    connect(shortcutMinus, &QShortcut::activated, this, &ValueDlg::on_zoomOut);

    m_queryXML = QString("");
    Options& options = Options::GetInstance();
    m_visualizationServiceEnable = options.getVisualizationServiceEnable();
    m_visualizationServiceURL = options.getVisualizationServiceURL();
    m_id = QString("");
    m_key = QString("");

    setStyleSheet(QString("QTextEdit { background-color: %1; }").arg(options.getBackgroundColor()));

    ui->prevButton->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/value-previous.png")));
    ui->nextButton->setIcon(QIcon(ThemeUtils::GetThemedIcon(":/value-next.png")));

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

QString ValueDlg::Process(QString in)
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
        word = word.toHtmlEscaped();
        // Use HTML safe spacing
        word.replace(" ", "&nbsp;");
        word.replace("\t", "&nbsp;&nbsp;&nbsp;&nbsp;");
        word.replace("\n", "<br>");
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

void ValueDlg::SetText(QString value, bool sqlHighlight)
{
    ///TODO: If we add multiple highlighters in the future, consider changing the sqlHighlight bool to an enum, that way we still only have 1 param for multiple syntax highlighters.
    QTextCursor * cursor = new QTextCursor(ui->textEdit->document());
    ui->textEdit->clear();

    if (sqlHighlight)
    {
        QString htmlText(QString("<body>") + Process(value) + QString("</body>"));
        cursor->insertHtml(htmlText);
    }
    else
    {
        cursor->insertText(value);
    }
    ui->textEdit->moveCursor(QTextCursor::Start);
    ui->textEdit->ensureCursorVisible();
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
    if (ui->wrapTextCheck->isChecked())
    {
        ui->textEdit->setLineWrapMode(QTextEdit::WidgetWidth);
    }
    else
    {
        ui->textEdit->setLineWrapMode(QTextEdit::NoWrap);
    }
}

void ValueDlg::SetQuery(QString queryXML)
{
    m_queryXML = queryXML;
    if (!m_queryXML.isEmpty())
    {
        ui->visualizeButton->setEnabled(true);
        ui->visualizeLabel->setText("");
    }
    else
    {
        ui->visualizeButton->setEnabled(false);
        ui->visualizeLabel->setText("Nothing to visualize");
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

    filePart.setBody(m_queryXML.toUtf8());

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

    QDesktopWidget widget;
    int screenId = widget.screenNumber(this);
    QRect screenRect = widget.availableGeometry(screenId);
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
    m_view->load(QUrl("qrc:///query-graphs/query-graphs.tlv.html?inline=" + QUrl::toPercentEncoding(m_queryXML)));
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
    settings.endGroup();
}

void ValueDlg::ReadSettings(QSettings& settings)
{
    settings.beginGroup("ValueDialog");
    sm_savedGeometry = settings.value("geometry").toByteArray();
    sm_savedFontPointSize = settings.value("fontPointSize").toReal();
    settings.endGroup();
}
