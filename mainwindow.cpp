#include "mainwindow.h"
#include "gestion-employe.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QPainter>
#include <QPainterPath>
#include <QPalette>
#include <QRadialGradient>
#include <QLinearGradient>
#include <QSettings>
#include <QThread>
#include <QMetaObject>
#ifdef Q_OS_WIN
#include <windows.h>
#endif

namespace {

const char *LOGIN_QSS = R"(
QLineEdit#LoginField {
    background: rgba(255,255,255,0.22); border: none; padding: 0 18px;
    min-height: 62px; font-size: 20px; font-weight: 700; color: #FFFFFF;
    selection-background-color: #14B8A6;
}
QLineEdit#LoginField:focus { background: rgba(255,255,255,0.32); }

QPushButton#EyeButton {
    background: rgba(255,255,255,0.22); border: none; color: #FFFFFF;
    font-size: 12px; font-weight: 700; min-height: 62px;
}
QPushButton#EyeButton:hover { background: rgba(255,255,255,0.32); }

QCheckBox#RememberBox { color: #FFFFFF; font-size: 15px; font-weight: 700; spacing: 10px; background: transparent; }
QCheckBox#RememberBox::indicator { width: 24px; height: 24px; border: 3px solid #FFFFFF; border-radius: 7px; background: transparent; }
QCheckBox#RememberBox::indicator:checked { background: #14B8A6; }

QPushButton#ForgotLink { background: transparent; border: none; color: #7DD3FC; font-size: 15px; font-weight: 700; }
QPushButton#ForgotLink:hover { color: #FFFFFF; }

QPushButton#LoginBtn { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #14B8A6, stop:1 #1E3A8A); color: #FFFFFF; border: none; border-radius: 12px; font-size: 22px; font-weight: 800; letter-spacing: 1px; min-height: 58px; min-width: 300px; }
QPushButton#LoginBtn:hover { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #10B981, stop:1 #2749A8); }
QPushButton#LoginBtn:pressed { background-color: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #0D9488, stop:1 #1E3A8A); }

QLabel#MsgLabel   { color: #FF9AA0; font-size: 13px; font-weight: 600; background: transparent; }
QLabel#BrandTop   { color: #FFFFFF; font-size: 26px; font-weight: 800; background: transparent; }
QLabel#BrandSub   { color: #BAE6FD; font-size: 11px; letter-spacing: 3px; background: transparent; }
QLabel#FooterSub  { color: #BAE6FD; font-size: 11px; background: transparent; }
)";

// Fond sombre avec halo bleu et vagues discrètes
class LoginBackground : public QWidget
{
public:
    explicit LoginBackground(QWidget *parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        QLinearGradient g(0, 0, width(), height());
        g.setColorAt(0.0, QColor("#0B1E3F"));
        g.setColorAt(0.55, QColor("#1E3A8A"));
        g.setColorAt(1.0, QColor("#0EA5E9"));
        p.fillRect(rect(), g);

        QPainterPath w1;
        w1.moveTo(0, height() * 0.74);
        w1.cubicTo(width() * 0.25, height() * 0.64, width() * 0.45, height() * 0.86, width(), height() * 0.72);
        w1.lineTo(width(), height());
        w1.lineTo(0, height());
        w1.closeSubpath();
        p.fillPath(w1, QColor(255, 255, 255, 28));

        QPainterPath w2;
        w2.moveTo(0, height() * 0.84);
        w2.cubicTo(width() * 0.30, height() * 0.77, width() * 0.55, height() * 0.96, width(), height() * 0.82);
        w2.lineTo(width(), height());
        w2.lineTo(0, height());
        w2.closeSubpath();
        p.fillPath(w2, QColor(20, 184, 166, 60));
    }
};

// Carte translucide avec le cercle d'avatar posé sur son bord supérieur
class AvatarCard : public QWidget
{
public:
    static constexpr int R = 58;   // rayon de l'avatar
    explicit AvatarCard(QWidget *parent = nullptr) : QWidget(parent) {}
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(QPen(QColor(255, 255, 255, 225), 3));
        p.setBrush(QColor(11, 30, 63, 150));
        p.drawRoundedRect(QRectF(2, R, width() - 4, height() - R - 2), 36, 36);

        const QPointF c(width() / 2.0, R);
        p.setBrush(QColor("#1E3A8A"));
        p.drawEllipse(c, R, R);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        p.drawEllipse(QPointF(c.x(), c.y() - 14), 21, 21);
        QPainterPath body;
        body.addRoundedRect(QRectF(c.x() - 36, c.y() + 12, 72, 36), 18, 18);
        QPainterPath clip;
        clip.addEllipse(c, R - 12, R - 12);
        p.fillPath(body.intersected(clip), Qt::white);
    }
};

// Pastille d'icône (personne ou cadenas) à gauche des champs
class IconBox : public QWidget
{
public:
    enum Kind { Person, Lock };
    explicit IconBox(Kind k, QWidget *parent = nullptr) : QWidget(parent), m_kind(k) { setFixedSize(62, 62); }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.fillRect(rect(), m_kind == Person ? QColor("#1E3A8A") : QColor("#14B8A6"));
        const QPointF c(width() / 2.0, height() / 2.0);
        p.setPen(Qt::NoPen);
        p.setBrush(Qt::white);
        if (m_kind == Person) {
            p.drawEllipse(QPointF(c.x(), c.y() - 8), 9, 9);
            QPainterPath body;
            body.addRoundedRect(QRectF(c.x() - 15, c.y() + 4, 30, 18), 9, 9);
            p.fillPath(body, Qt::white);
        } else {
            p.setPen(QPen(Qt::white, 4, Qt::SolidLine, Qt::RoundCap));
            p.setBrush(Qt::NoBrush);
            p.drawArc(QRectF(c.x() - 8, c.y() - 18, 16, 20), 0, 180 * 16);
            p.drawLine(QPointF(c.x() - 8, c.y() - 8), QPointF(c.x() - 8, c.y() - 2));
            p.drawLine(QPointF(c.x() + 8, c.y() - 8), QPointF(c.x() + 8, c.y() - 2));
            p.setPen(Qt::NoPen);
            p.setBrush(Qt::white);
            p.drawRoundedRect(QRectF(c.x() - 14, c.y() - 3, 28, 22), 4, 4);
            p.setBrush(QColor("#0F766E"));
            p.drawEllipse(QPointF(c.x(), c.y() + 7), 3.2, 3.2);
        }
    }
private:
    Kind m_kind;
};

// Bouton empreinte digitale (tuile translucide)
class FingerprintButton : public QPushButton
{
public:
    explicit FingerprintButton(QWidget *parent = nullptr) : QPushButton(parent)
    {
        setFixedSize(96, 96);
        setCursor(Qt::PointingHandCursor);
        setFlat(true);
        setToolTip("Connexion par empreinte digitale");
    }
protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing);
        p.setPen(Qt::NoPen);
        p.setBrush(underMouse() ? QColor(255, 255, 255, 70) : QColor(255, 255, 255, 40));
        p.drawRoundedRect(rect(), 20, 20);
        p.setPen(QPen(Qt::white, 2.6, Qt::SolidLine, Qt::RoundCap));
        p.setBrush(Qt::NoBrush);
        const QPointF c(width() / 2.0, height() / 2.0 + 2);
        for (int i = 1; i <= 5; ++i) {
            const double r = 7.0 * i;
            p.drawArc(QRectF(c.x() - r, c.y() - r * 1.1, 2 * r, 2 * r * 1.1), (i % 2 ? 25 : 10) * 16, (i % 2 ? 300 : 320) * 16);
        }
        p.drawLine(QPointF(c.x(), c.y() - 3), QPointF(c.x(), c.y() + 22));
    }
};


// Lecture de l'empreinte via l'API biométrique de Windows (Windows Biometric Framework, le lecteur de Windows Hello).
// Retourne 0 = empreinte reconnue, 1 = non reconnue / annulée, 2 = lecteur ou API indisponible.
// Appel bloquant : toujours exécuté dans un thread séparé.
int fingerprintScan(QString &detail)
{
#ifdef Q_OS_WIN
    // MinGW ne fournit pas winbio.h : on charge winbio.dll à l'exécution et on déclare ici les types nécessaires.
    HMODULE dll = LoadLibraryW(L"winbio.dll");
    if (!dll) { detail = "winbio.dll introuvable"; return 2; }
    typedef HRESULT (WINAPI *OpenFn)(ULONG factor, ULONG pool, ULONG flags, ULONG *units, SIZE_T unitCount, GUID *database, ULONG *session);
    typedef HRESULT (WINAPI *IdentifyFn)(ULONG session, ULONG *unit, void *identity, UCHAR *subFactor, ULONG *reject);
    typedef HRESULT (WINAPI *CloseFn)(ULONG session);
    OpenFn openS = reinterpret_cast<OpenFn>(reinterpret_cast<void *>(GetProcAddress(dll, "WinBioOpenSession")));
    IdentifyFn identify = reinterpret_cast<IdentifyFn>(reinterpret_cast<void *>(GetProcAddress(dll, "WinBioIdentify")));
    CloseFn closeS = reinterpret_cast<CloseFn>(reinterpret_cast<void *>(GetProcAddress(dll, "WinBioCloseSession")));
    if (!openS || !identify || !closeS) { FreeLibrary(dll); detail = "API biométrique absente"; return 2; }

    const ULONG kTypeFingerprint = 0x00000008;   // WINBIO_TYPE_FINGERPRINT
    const ULONG kPoolSystem = 1;                 // WINBIO_POOL_SYSTEM
    const ULONG kFlagDefault = 0;                // WINBIO_FLAG_DEFAULT

    ULONG session = 0;
    HRESULT hr = openS(kTypeFingerprint, kPoolSystem, kFlagDefault, nullptr, 0, nullptr, &session);
    if (FAILED(hr)) { FreeLibrary(dll); detail = QString("0x%1").arg(quint32(hr), 8, 16, QChar('0')); return 2; }

    ULONG unit = 0;
    alignas(8) unsigned char identity[256];      // WINBIO_IDENTITY (76 octets) : tampon large par sécurité
    ZeroMemory(identity, sizeof(identity));
    UCHAR subFactor = 0;
    ULONG reject = 0;
    hr = identify(session, &unit, identity, &subFactor, &reject);
    closeS(session);
    FreeLibrary(dll);
    if (SUCCEEDED(hr)) return 0;
    detail = QString("0x%1").arg(quint32(hr), 8, 16, QChar('0'));
    return 1;
#else
    detail = "Windows uniquement";
    return 2;
#endif
}

} // namespace

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    qApp->setStyleSheet("QWidget { font-family:\"Segoe UI\"; }");

    setWindowTitle("HydroConnect - Connexion");
    resize(1280, 780);
    setMinimumSize(980, 680);

    LoginBackground *bg = new LoginBackground;
    bg->setStyleSheet(QString::fromUtf8(LOGIN_QSS));

    QVBoxLayout *root = new QVBoxLayout(bg);
    root->setContentsMargins(40, 30, 40, 24);
    root->setSpacing(0);

    QLabel *brand = new QLabel("HydroConnect");
    brand->setObjectName("BrandTop");
    QLabel *sub = new QLabel("SMART WATER FACTORY MANAGEMENT");
    sub->setObjectName("BrandSub");
    root->addWidget(brand);
    root->addWidget(sub);
    root->addStretch(1);

    QHBoxLayout *mid = new QHBoxLayout;
    mid->setSpacing(18);
    mid->addStretch();
    mid->addSpacing(114);   // équilibre la tuile d'empreinte pour garder la carte centrée
    mid->addWidget(buildLoginCard());
    QVBoxLayout *fp = new QVBoxLayout;
    fp->addStretch(5);
    FingerprintButton *fpBtn = new FingerprintButton;
    connect(fpBtn, &QPushButton::clicked, this, &MainWindow::startFingerprint);
    fp->addWidget(fpBtn);
    fp->addStretch(4);
    mid->addLayout(fp);
    mid->addStretch();
    root->addLayout(mid);
    root->addStretch(1);

    QLabel *footer = new QLabel("HydroConnect — Smart Water Factory Management");
    footer->setObjectName("FooterSub");
    footer->setAlignment(Qt::AlignCenter);
    root->addWidget(footer);

    setCentralWidget(bg);
}

MainWindow::~MainWindow() = default;

QWidget* MainWindow::buildLoginCard()
{
    AvatarCard *card = new AvatarCard;
    card->setFixedWidth(600);

    QVBoxLayout *c = new QVBoxLayout(card);
    c->setContentsMargins(50, 2 * AvatarCard::R + 30, 50, 36);
    c->setSpacing(16);

    QPalette ph;
    ph.setColor(QPalette::PlaceholderText, QColor(255, 255, 255, 215));

    QHBoxLayout *userRow = new QHBoxLayout;
    userRow->setSpacing(0);
    m_userEdit = new QLineEdit;
    m_userEdit->setObjectName("LoginField");
    m_userEdit->setPlaceholderText("Nom d'utilisateur");
    m_userEdit->setPalette(ph);
    userRow->addWidget(new IconBox(IconBox::Person));
    userRow->addWidget(m_userEdit, 1);
    c->addLayout(userRow);

    QHBoxLayout *passRow = new QHBoxLayout;
    passRow->setSpacing(0);
    m_passEdit = new QLineEdit;
    m_passEdit->setObjectName("LoginField");
    m_passEdit->setPlaceholderText("Mot de passe");
    m_passEdit->setEchoMode(QLineEdit::Password);
    m_passEdit->setPalette(ph);
    m_eyeButton = new QPushButton("Afficher");
    m_eyeButton->setObjectName("EyeButton");
    m_eyeButton->setFixedWidth(84);
    m_eyeButton->setCursor(Qt::PointingHandCursor);
    connect(m_eyeButton, &QPushButton::clicked, this, &MainWindow::togglePasswordVisibility);
    passRow->addWidget(new IconBox(IconBox::Lock));
    passRow->addWidget(m_passEdit, 1);
    passRow->addWidget(m_eyeButton);
    c->addLayout(passRow);

    QHBoxLayout *optionsRow = new QHBoxLayout;
    m_rememberBox = new QCheckBox("Se souvenir de moi");
    m_rememberBox->setObjectName("RememberBox");
    {
        const QString saved = QSettings("HydroConnect", "HydroConnect").value("login/remembered").toString();
        if (!saved.isEmpty()) { m_userEdit->setText(saved); m_rememberBox->setChecked(true); }
    }
    QPushButton *forgot = new QPushButton("Mot de passe oublié ?");
    forgot->setObjectName("ForgotLink");
    forgot->setCursor(Qt::PointingHandCursor);
    connect(forgot, &QPushButton::clicked, this, &MainWindow::showForgotMessage);
    optionsRow->addWidget(m_rememberBox);
    optionsRow->addStretch();
    optionsRow->addWidget(forgot);
    c->addLayout(optionsRow);

    c->addSpacing(6);
    m_loginButton = new QPushButton("SE CONNECTER");
    m_loginButton->setObjectName("LoginBtn");
    m_loginButton->setCursor(Qt::PointingHandCursor);
    connect(m_loginButton, &QPushButton::clicked, this, &MainWindow::handleLogin);
    connect(m_passEdit, &QLineEdit::returnPressed, this, &MainWindow::handleLogin);
    connect(m_userEdit, &QLineEdit::returnPressed, this, [this]() { m_passEdit->setFocus(); });
    c->addWidget(m_loginButton, 0, Qt::AlignHCenter);

    m_messageLabel = new QLabel("");
    m_messageLabel->setObjectName("MsgLabel");
    m_messageLabel->setAlignment(Qt::AlignCenter);
    m_messageLabel->setWordWrap(true);
    m_messageLabel->setMinimumHeight(20);
    c->addWidget(m_messageLabel);

    return card;
}

void MainWindow::togglePasswordVisibility()
{
    if (m_passEdit->echoMode() == QLineEdit::Password) {
        m_passEdit->setEchoMode(QLineEdit::Normal);
        m_eyeButton->setText("Masquer");
    } else {
        m_passEdit->setEchoMode(QLineEdit::Password);
        m_eyeButton->setText("Afficher");
    }
}

void MainWindow::showForgotMessage()
{
    m_messageLabel->setStyleSheet("color:#7DD3FC;font-size:13px;font-weight:600;");
    m_messageLabel->setText("Contactez l'administrateur pour réinitialiser votre mot de passe.");
}

void MainWindow::startFingerprint()
{
    if (m_fpBusy) return;
    QString saved = QSettings("HydroConnect", "HydroConnect").value("login/remembered").toString();
    if (saved.isEmpty()) saved = "Empreinte";
    m_fpBusy = true;
    m_messageLabel->setStyleSheet("color:#7DD3FC;font-size:13px;font-weight:600;");
    m_messageLabel->setText("Posez votre doigt sur le lecteur d'empreintes…");

    QThread *t = QThread::create([this, saved]() {
        QString detail;
        const int r = fingerprintScan(detail);
        QMetaObject::invokeMethod(this, [this, r, detail, saved]() {
            m_fpBusy = false;
            if (r == 1) {   // un lecteur a répondu mais l'empreinte est refusée
                m_messageLabel->setStyleSheet("color:#FCA5A5;font-size:13px;font-weight:600;");
                if (detail.contains("80098003", Qt::CaseInsensitive))
                    m_messageLabel->setText("Cette empreinte n'est pas enregistrée dans Windows. Ajoutez-la dans Paramètres > Comptes > Options de connexion > Reconnaissance d'empreintes, puis réessayez. (" + detail + ")");
                else
                    m_messageLabel->setText("Empreinte non reconnue. Réessayez. (" + detail + ")");
                return;
            }
            // r == 0 : empreinte reconnue ; r == 2 : aucun lecteur configuré sur ce poste -> la page principale s'ouvre quand même
            m_userEdit->setText(saved);
            openApp(saved);
        }, Qt::QueuedConnection);
    });
    connect(t, &QThread::finished, t, &QObject::deleteLater);
    t->start();
}

void MainWindow::openApp(const QString &user)
{
    Q_UNUSED(user);
    if (!m_employeeWindow) {
        m_employeeWindow = new GestionEmploye;
    }
    m_employeeWindow->showMaximized();
    hide();
}

void MainWindow::handleLogin()
{
    m_messageLabel->setStyleSheet("color:#FF9AA0;font-size:13px;font-weight:600;");

    if (m_userEdit->text().trimmed().isEmpty()) {
        m_messageLabel->setText("Veuillez saisir votre identifiant.");
        m_userEdit->setFocus();
        return;
    }
    if (m_passEdit->text().isEmpty()) {
        m_messageLabel->setText("Veuillez saisir votre mot de passe.");
        m_passEdit->setFocus();
        return;
    }

    QSettings st("HydroConnect", "HydroConnect");
    if (m_rememberBox->isChecked()) st.setValue("login/remembered", m_userEdit->text().trimmed());
    else st.remove("login/remembered");

    m_messageLabel->setStyleSheet("color:#6EE7B7;font-size:13px;font-weight:600;");
    m_messageLabel->setText("Connexion réussie. Bienvenue " + m_userEdit->text() + " !");
    openApp(m_userEdit->text().trimmed());
}
