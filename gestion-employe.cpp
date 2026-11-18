#include "gestion-employe.h"
#include "logo-data.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFrame>
#include <QHeaderView>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPixmap>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDateEdit>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QStyle>
#include <QProgressBar>
#include <QMessageBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QAbstractItemView>
#include <QRegularExpression>
#include <QColor>
#include <QPen>
#include <QBrush>
#include <QDate>
#include <algorithm>
#include <cmath>
#include "connection.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

namespace {
QStringList postes(){ return {"Ingénieur","Technicien","Responsable RH","Comptable"}; }
QStringList departements(){ return {"Maintenance","Production","Laboratoire","Usage Merchandiser Technique"}; }
bool validEmail(const QString &s){ return QRegularExpression("^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[A-Za-z]{2,}$").match(s).hasMatch(); }
QPixmap logo(){ QPixmap p; p.loadFromData(kLogoPng,sizeof(kLogoPng),"PNG"); return p; }
}

namespace {
// Message lisible pour les erreurs Oracle les plus courantes, suivi du vrai message Oracle
// ID_USINE : valeur NULL si aucune usine n'est associée
QVariant usineValue(int id){return id>0?QVariant(id):QVariant(QMetaType(QMetaType::Int));}

QString oracleMessage(const QSqlError &er){
    const QString t=er.text();QString hint;
    if(t.contains("ORA-00001"))hint="Doublon : cet ID (ou une valeur unique) existe déjà dans la base.";
    else if(t.contains("ORA-02290"))hint="Une valeur ne respecte pas les contraintes de la table (âge, email...).";
    else if(t.contains("ORA-01400"))hint="ID_USINE est obligatoire dans votre table : ajoutez une usine dans USINE, ou rendez la colonne facultative (ALTER TABLE EMPLOYE MODIFY ID_USINE NULL).";
    else if(t.contains("ORA-02291"))hint="Usine inexistante : choisissez une usine de la liste.";
    else if(t.contains("ORA-02292"))hint="Suppression impossible : cet employé est référencé par une autre table.";
    else if(t.contains("ORA-12899"))hint="Une valeur est trop longue pour sa colonne.";
    else if(t.contains("ORA-00942"))hint="Table EMPLOYE introuvable pour cet utilisateur : vérifiez le compte dans connection.cpp.";
    else if(t.contains("ORA-00904"))hint="Colonne introuvable : colonne manquante : exécutez sql/adaptation_table_employe.sql.";
    return hint.isEmpty()?t:hint+"\n\nMessage Oracle :\n"+t;
}
}

BarChart::BarChart(QWidget *parent):QWidget(parent){ setMinimumHeight(145); }
void BarChart::setData(const QVector<double>&v,const QVector<QString>&l,const QString&t){values=v;labels=l;title=t;update();}
void BarChart::paintEvent(QPaintEvent*){
    QPainter p(this); p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QColor("#173A60")); p.setFont(QFont("Arial",10,QFont::Bold)); p.drawText(12,20,title);
    if(values.isEmpty()) return;
    int L=35,R=10,T=35,B=30,w=width()-L-R,h=height()-T-B; double mx=1;
    for(double v:values) mx=qMax(mx,v); if(mx<=0)mx=1;
    p.setPen(QColor("#E4EAF1")); for(int i=0;i<=4;i++){int y=T+h-i*h/4;p.drawLine(L,y,width()-R,y);}
    int n=values.size(); int bw=qMax(12,w/(n*2));
    for(int i=0;i<n;i++){
        int x=L+(i+0.5)*w/n-bw/2; int bh=(int)(values[i]/mx*(h-5));
        p.setBrush(QColor("#287CE2"));p.setPen(Qt::NoPen);p.drawRoundedRect(x,T+h-bh,bw,bh,4,4);
        p.setPen(QColor("#173A60"));p.setFont(QFont("Arial",8,QFont::Bold));p.drawText(x-15,T+h-bh-4,bw+30,16,Qt::AlignCenter,QString::number((int)values[i]));
        p.setFont(QFont("Arial",7));p.drawText(x-35,T+h+4,70,22,Qt::AlignCenter,labels.value(i));
    }
}

EmployePieChart::EmployePieChart(QWidget *parent):QWidget(parent){setMinimumHeight(145);}
void EmployePieChart::setData(const QVector<double>&v,const QVector<QString>&l){values=v;labels=l;update();}
void EmployePieChart::paintEvent(QPaintEvent*){
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);
    p.setPen(QColor("#173A60"));p.setFont(QFont("Arial",10,QFont::Bold));p.drawText(12,20,"Répartition des employés");
    if(values.isEmpty())return;
    double total=0;for(double v:values)total+=v;if(total<=0)return;
    QRectF r(12,38,92,92);QVector<QColor> colors={QColor("#287CE2"),QColor("#20B486"),QColor("#FF8A34"),QColor("#8B6FE8")};
    int start=0;
    for(int i=0;i<values.size();i++){int span=qRound(5760*values[i]/total);p.setBrush(colors.value(i,QColor("#287CE2")));p.setPen(Qt::NoPen);p.drawPie(r,start,span);start+=span;}
    int y=48;for(int i=0;i<labels.size();i++){p.setBrush(colors.value(i,QColor("#287CE2")));p.drawEllipse(125,y,9,9);p.setPen(QColor("#526173"));p.setFont(QFont("Arial",8));p.drawText(140,y-3,150,18,Qt::AlignLeft|Qt::AlignVCenter,labels[i]+" : "+QString::number((int)values[i]));y+=22;}
}

EmployeBottleWidget::EmployeBottleWidget(QWidget *parent):QWidget(parent){setMinimumHeight(130);}
void EmployeBottleWidget::paintEvent(QPaintEvent*){
    QPainter p(this);p.setRenderHint(QPainter::Antialiasing);int x=width()/2-38;
    p.setBrush(QColor("#54A9E8"));p.setPen(QColor("#78C7F2"));p.drawRoundedRect(x,35,76,85,22,22);p.drawRoundedRect(x+18,18,40,24,8,8);
    p.setBrush(QColor("#BFE9FF"));p.setPen(Qt::NoPen);p.drawRoundedRect(x+15,53,12,52,6,6);
    p.setBrush(QColor("#E8F7FF"));p.drawEllipse(x+50,88,8,8);p.drawEllipse(x+33,102,5,5);
}

GestionEmploye::GestionEmploye(QWidget *parent):QMainWindow(parent){
    setMinimumSize(1100,700);
    resize(1360,820);
    setupStyle();
    setupUI();
    loadUsines();
    reload();
}

void GestionEmploye::setupStyle(){setStyleSheet(
"QMainWindow{background:#F3F7FC;} QWidget{font-family:'Segoe UI',Arial;color:#17324F;} "
"QFrame#side{background:#102D50;} QFrame#panel{background:white;border:1px solid #E4ECF5;border-radius:15px;} "
"QLabel#title{font-size:25px;font-weight:800;color:#15385E;} QLabel#sub{font-size:12px;color:#7A8BA0;} "
"QLabel#logoText{font-size:21px;font-weight:800;color:white;} "
"QPushButton#menu{background:transparent;color:#C6D6E8;border:none;text-align:left;padding:10px 13px;font-size:13px;font-weight:600;border-radius:9px;} "
"QPushButton#menu:hover{background:#1B426D;color:white;} "
"QPushButton#active{background:#3478E5;color:white;border:none;text-align:left;padding:10px 13px;font-size:13px;font-weight:700;border-radius:9px;} "
"QLineEdit,QComboBox,QSpinBox,QDateEdit{background:#FBFDFF;border:1px solid #D9E4F0;border-radius:8px;padding:5px 9px;min-height:20px;color:#17324F;selection-background-color:#3478E5;selection-color:white;} "
"QLineEdit:focus,QComboBox:focus,QSpinBox:focus,QDateEdit:focus{background:white;border:1px solid #3478E5;} "
"QPushButton{border:none;border-radius:9px;padding:8px 14px;font-weight:700;} "
"QPushButton#add,QPushButton#green{background:#3478E5;color:white;} QPushButton#add:hover,QPushButton#green:hover{background:#2868CF;} "
"QPushButton#edit{background:#EAF2FF;color:#2868CF;border:1px solid #D8E6FA;} QPushButton#edit:hover{background:#DDEAFF;} "
"QPushButton#delete{background:#FFF7F6;color:#B84B43;border:1px solid #F2DEDC;} QPushButton#delete:hover{background:#FCEBE9;} "
"QPushButton#normal{background:#EDF4FC;color:#2868CF;} QPushButton#normal:hover{background:#DFEBFA;} "
"QTableWidget{background:white;alternate-background-color:#F7FAFE;border:none;gridline-color:#EDF2F8;selection-background-color:#E5F0FF;selection-color:#17324F;} "
"QTableWidget::item{padding:5px 7px;border-bottom:1px solid #EFF3F8;} "
"QHeaderView::section{background:#EEF4FB;color:#23486F;padding:9px 6px;border:none;font-weight:700;} "
"QProgressBar{background:#E6EEFA;border:none;border-radius:7px;text-align:center;color:#17324F;} QProgressBar::chunk{background:#3478E5;border-radius:7px;}");}

QWidget *GestionEmploye::createSidebar(){
    QFrame *side=new QFrame;side->setObjectName("side");side->setFixedWidth(218);QVBoxLayout *v=new QVBoxLayout(side);v->setContentsMargins(13,20,13,13);v->setSpacing(5);
    QLabel *img=new QLabel;img->setPixmap(logo().scaled(132,96,Qt::KeepAspectRatio,Qt::SmoothTransformation));img->setAlignment(Qt::AlignCenter);v->addWidget(img);
    QLabel *lt=new QLabel("HydroConnect");lt->setObjectName("logoText");lt->setAlignment(Qt::AlignCenter);v->addWidget(lt);v->addSpacing(16);
    QStringList m={"🏠   Accueil","👥   Employés","🏭   Usine","📊   Statistiques","📄   Exportation","⚙   Paramètres","🤖   Chatbot"};
    for(int i=0;i<m.size();i++){
        QPushButton *b=new QPushButton(m[i]);
        b->setFixedHeight(39);
        b->setObjectName(i==1?"active":"menu");
        v->addWidget(b);

        // Move the selected-state highlight to whichever sidebar item is clicked.
        connect(b, &QPushButton::clicked, this, [this, b]() {
            const auto buttons = findChildren<QPushButton*>();
            for (QPushButton *item : buttons) {
                if (item->objectName() == "menu" || item->objectName() == "active") {
                    item->setObjectName(item == b ? "active" : "menu");
                    item->style()->unpolish(item);
                    item->style()->polish(item);
                    item->update();
                }
            }
        });

        // Keep the HydroConnect sidebar and swap only the main content.
        if (i == 1) {
            connect(b, &QPushButton::clicked, this, [this]() {
                if (m_contentStack) m_contentStack->setCurrentWidget(m_employeeContent);
            });
        } else if (i == 0 || i == 1) {
            connect(b, &QPushButton::clicked, this, [this]() {
                if (m_contentStack && m_employeeContent) m_contentStack->setCurrentWidget(m_employeeContent);
            });
        } else if (i == 3) {
            // Les statistiques restent visibles à droite du tableau, dans la page Employés.
            connect(b, &QPushButton::clicked, this, [this]() {
                if (m_contentStack && m_employeeContent)
                    m_contentStack->setCurrentWidget(m_employeeContent);
                updateStatistics();
                updateCharts();
            });
        } else if (i == 4) {
            connect(b, &QPushButton::clicked, this, &GestionEmploye::exportEmployes);
        } else if (i == 6) {
            // Le chatbot s'ouvre dans la page Employés, sans changer de fenêtre.
            connect(b, &QPushButton::clicked, this, [this]() {
                if (m_contentStack && m_employeeContent)
                    m_contentStack->setCurrentWidget(m_employeeContent);
                if (m_chatbotContent) m_chatbotContent->setVisible(true);
            });
        }
    }
    v->addStretch();EmployeBottleWidget *bottle=new EmployeBottleWidget;v->addWidget(bottle);return side;
}

QWidget *GestionEmploye::createCard(const QString&t,const QString&val,const QString&icon,const QString&bg){
    QFrame *f=new QFrame;f->setObjectName("panel");QHBoxLayout *h=new QHBoxLayout(f);QLabel *ic=new QLabel(icon);ic->setAlignment(Qt::AlignCenter);ic->setFixedSize(58,58);ic->setStyleSheet("background:"+bg+";border-radius:12px;font-size:25px;");QVBoxLayout *v=new QVBoxLayout;QLabel *a=new QLabel(t);a->setStyleSheet("color:#687789;");QLabel *b=new QLabel(val);b->setStyleSheet("font-size:25px;font-weight:bold;color:#173A60;");v->addWidget(a);v->addWidget(b);h->addWidget(ic);h->addLayout(v);h->addStretch();return f;
}

QWidget *GestionEmploye::createTopCards(){
    QHBoxLayout *h=new QHBoxLayout;QWidget *w=new QWidget;w->setLayout(h);
    QWidget *c1=createCard("Total employés",QString::number(employes.size()),"👥","#E5F0FF");h->addWidget(c1);h->addStretch();
    totalLabel=c1->findChildren<QLabel*>().last();
    return w;
}

QPushButton *GestionEmploye::button(const QString&t,const QString&n){QPushButton*b=new QPushButton(t);b->setObjectName(n);return b;}

QWidget *GestionEmploye::createTablePanel(){
    QFrame *p=new QFrame;p->setObjectName("panel");QVBoxLayout*v=new QVBoxLayout(p);v->setContentsMargins(13,12,13,12);v->setSpacing(9);
    QHBoxLayout *top=new QHBoxLayout;top->setSpacing(7);QLabel*t=new QLabel("👥  Liste des employés");t->setStyleSheet("font-size:17px;font-weight:bold;color:#173A60;");searchEdit=new QLineEdit;searchEdit->setPlaceholderText("🔎 Rechercher ID, nom, prénom, email, poste ou département...");searchEdit->setMinimumWidth(160);searchEdit->setMaximumWidth(300);triCombo=new QComboBox;triCombo->addItems({"Aucun","Nom","Âge","Poste","Département"});departementFilter=new QComboBox;departementFilter->addItem("Tous");departementFilter->addItems(departements());top->addWidget(t);top->addStretch();top->addWidget(searchEdit);top->addWidget(new QLabel("Trier :"));top->addWidget(triCombo);top->addWidget(new QLabel("Département :"));top->addWidget(departementFilter);v->addLayout(top);
    table=new QTableWidget;table->setColumnCount(9);table->setHorizontalHeaderLabels({"ID","Nom","Prénom","Âge","Email","Téléphone","Poste","Département","Date d'embauche"});table->setSelectionBehavior(QAbstractItemView::SelectRows);table->setSelectionMode(QAbstractItemView::SingleSelection);table->setEditTriggers(QAbstractItemView::NoEditTriggers);table->verticalHeader()->setVisible(false);table->verticalHeader()->setDefaultSectionSize(34);table->horizontalHeader()->setStretchLastSection(true);table->setAlternatingRowColors(true);v->addWidget(table);
    connect(searchEdit, &QLineEdit::textChanged, this, &GestionEmploye::searchEmployes);
    connect(departementFilter, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GestionEmploye::searchEmployes);
    connect(triCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GestionEmploye::sortEmployes);
    connect(table, &QTableWidget::itemSelectionChanged, this, &GestionEmploye::selectEmploye);
    QHBoxLayout*bs=new QHBoxLayout;QPushButton*e=button("📄 Exporter Excel","normal");QPushButton*s=button("📊 Statistiques","green");bs->addStretch();bs->addWidget(e);bs->addWidget(s);v->addLayout(bs);
    connect(e, &QPushButton::clicked, this, &GestionEmploye::exportEmployes);
    connect(s, &QPushButton::clicked, this, &GestionEmploye::showStatistics);
    return p;
}

QWidget *GestionEmploye::createStatsPanel(){
    QFrame*p=new QFrame;p->setObjectName("panel");QVBoxLayout*v=new QVBoxLayout(p);v->setContentsMargins(13,12,13,12);v->setSpacing(8);QHBoxLayout*h=new QHBoxLayout;QLabel*t=new QLabel("📊  Statistiques");t->setStyleSheet("font-size:17px;font-weight:bold;color:#173A60;");statsMode=new QComboBox;statsMode->addItems({"Âge","Poste","Département"});h->addWidget(t);h->addStretch();h->addWidget(statsMode);v->addLayout(h);pieChart=new EmployePieChart;pieChart->setMinimumHeight(125);pieChart->setMaximumHeight(155);v->addWidget(pieChart);connect(statsMode,QOverload<int>::of(&QComboBox::currentIndexChanged),this,&GestionEmploye::updateCharts);return p;
}

QWidget *GestionEmploye::createFormPanel(){
    QFrame*p=new QFrame;p->setObjectName("panel");QVBoxLayout*v=new QVBoxLayout(p);v->setContentsMargins(15,11,15,11);v->setSpacing(7);QHBoxLayout*hh=new QHBoxLayout;QLabel*t=new QLabel("👤  Ajouter / Modifier un employé");t->setStyleSheet("font-size:17px;font-weight:bold;color:#173A60;");QPushButton*clear=button("Effacer","normal");hh->addWidget(t);hh->addStretch();hh->addWidget(clear);v->addLayout(hh);
    QGridLayout*g=new QGridLayout;g->setHorizontalSpacing(12);g->setVerticalSpacing(6);idEdit=new QLineEdit;nomEdit=new QLineEdit;prenomEdit=new QLineEdit;ageSpin=new QSpinBox;ageSpin->setRange(18,100);emailEdit=new QLineEdit;telephoneEdit=new QLineEdit;posteCombo=new QComboBox;posteCombo->addItems(postes());departementCombo=new QComboBox;departementCombo->addItems(departements());dateEdit=new QDateEdit(QDate::currentDate());dateEdit->setCalendarPopup(true);dateEdit->setDisplayFormat("dd/MM/yyyy");
    g->addWidget(new QLabel("ID employé"),0,0);g->addWidget(idEdit,0,1);g->addWidget(new QLabel("Nom"),0,2);g->addWidget(nomEdit,0,3);g->addWidget(new QLabel("Prénom"),1,0);g->addWidget(prenomEdit,1,1);g->addWidget(new QLabel("Âge"),1,2);g->addWidget(ageSpin,1,3);g->addWidget(new QLabel("Email"),2,0);g->addWidget(emailEdit,2,1);g->addWidget(new QLabel("Téléphone"),2,2);g->addWidget(telephoneEdit,2,3);g->addWidget(new QLabel("Poste"),3,0);g->addWidget(posteCombo,3,1);g->addWidget(new QLabel("Département"),3,2);g->addWidget(departementCombo,3,3);g->addWidget(new QLabel("Date d'embauche"),4,0);g->addWidget(dateEdit,4,1);v->addLayout(g);
    QHBoxLayout*b=new QHBoxLayout;QPushButton*a=button("＋ Ajouter","add");QPushButton*m=button("✎ Modifier","edit");QPushButton*d=button("🗑 Supprimer","delete");b->addWidget(a);b->addWidget(m);b->addWidget(d);b->addStretch();v->addLayout(b);
    connect(a, &QPushButton::clicked, this, &GestionEmploye::addEmploye);
    connect(m, &QPushButton::clicked, this, &GestionEmploye::modifyEmploye);
    connect(d, &QPushButton::clicked, this, &GestionEmploye::deleteEmploye);
    connect(clear, &QPushButton::clicked, this, &GestionEmploye::clearForm);
    return p;
}

QWidget *GestionEmploye::createBottomPanels(){
    return createFormPanel();
}

void GestionEmploye::setupUI(){
    QWidget *c = new QWidget;
    setCentralWidget(c);
    QHBoxLayout *root = new QHBoxLayout(c);
    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);
    root->addWidget(createSidebar());

    m_contentStack = new QStackedWidget;
    // Page Employés directement intégrée au QStackedWidget : aucun QScrollArea.
    m_employeeContent = new QWidget;
    m_employeeContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *v = new QVBoxLayout(m_employeeContent);
    v->setContentsMargins(18, 14, 18, 14);
    v->setSpacing(9);

    QLabel *t = new QLabel("👥 Gestion des Employés");
    t->setObjectName("title");
    QLabel *sub = new QLabel("Gérez les informations des employés de HydroConnect depuis un formulaire unique.");
    sub->setObjectName("sub");
    v->addWidget(t);
    v->addWidget(sub);
    v->addWidget(createFormPanel());

    QHBoxLayout *mid = new QHBoxLayout;
    QWidget *tablePanel = createTablePanel();
    QWidget *statsPanel = createStatsPanel();
    statsPanel->setMinimumWidth(260);
    statsPanel->setMaximumWidth(310);
    mid->setSpacing(10);
    mid->addWidget(tablePanel, 5);
    mid->addWidget(statsPanel, 2);
    v->addLayout(mid, 1);

    // HydroBot est intégré à la même page que le formulaire, le tableau et les statistiques.
    QPushButton *chatBanner = new QPushButton("🤖   Ouvrir HydroBot   —   Assistant HydroConnect");
    chatBanner->setMinimumHeight(38);
    chatBanner->setMaximumHeight(40);
    chatBanner->setStyleSheet("QPushButton{background:white;border:1px solid #CFE3F8;border-radius:14px;text-align:left;padding:12px;font-size:15px;font-weight:700;color:#173A60;} QPushButton:hover{background:#EAF2FF;}");
    v->addWidget(chatBanner);

    m_chatbotContent = createChatbotPage();
    m_chatbotContent->setVisible(false);
    m_chatbotContent->setMaximumHeight(205);
    v->addWidget(m_chatbotContent);
    connect(chatBanner, &QPushButton::clicked, this, [this, chatBanner]() {
        const bool visible = m_chatbotContent && m_chatbotContent->isVisible();
        if (m_chatbotContent) m_chatbotContent->setVisible(!visible);
        chatBanner->setText(visible
            ? "🤖   Ouvrir HydroBot   —   Assistant HydroConnect"
            : "🤖   Masquer HydroBot   —   Assistant HydroConnect");
    });

    m_contentStack->addWidget(m_employeeContent);
    root->addWidget(m_contentStack, 1);
}

void GestionEmploye::loadUsines(){
    // ID_USINE est obligatoire (clé étrangère) : les nouveaux employés reçoivent l'usine par défaut
    Connection &c=Connection::createInstance();
    if(!c.ensureOpen())return;                            // l'erreur de connexion est signalée par loadFromDatabase()
    QSqlQuery q(c.database());
    if(!q.exec("SELECT MIN(ID_USINE) FROM USINE")){QMessageBox::warning(this,"Usines","Lecture de la table USINE impossible :\n"+oracleMessage(q.lastError()));return;}
    defaultUsineId=(q.next()&&!q.value(0).isNull())?q.value(0).toInt():0;
    if(defaultUsineId<=0){QSqlQuery a(c.database());a.exec("ALTER TABLE EMPLOYE MODIFY ID_USINE NULL");}   // USINE est vide : ID_USINE devient facultatif (erreur ORA-01451 ignorée si c'est déjà le cas)
}

bool GestionEmploye::loadFromDatabase(){
    Connection &c=Connection::createInstance();
    if(!c.ensureOpen()){QMessageBox::critical(this,"Base de données","Connexion Oracle impossible :\n"+c.lastErrorText());return false;}
    QString order="ID_EMPLOYE";const QString s=triCombo?triCombo->currentText():"Aucun";
    if(s=="Nom")order="UPPER(NOM), UPPER(PRENOM)";else if(s=="Âge")order="AGE, UPPER(NOM)";else if(s=="Poste")order="UPPER(POSTE), UPPER(NOM)";else if(s=="Département")order="UPPER(DEPARTEMENT), UPPER(NOM)";
    QSqlQuery q(c.database());
    if(!q.exec("SELECT ID_EMPLOYE, NOM, PRENOM, AGE, EMAIL, TELEPHONE, POSTE, DEPARTEMENT, DATE_EMBAUCHE, ID_USINE FROM EMPLOYE ORDER BY "+order)){
        QMessageBox::critical(this,"Base de données","Lecture des employés impossible :\n"+oracleMessage(q.lastError()));return false;}
    employes.clear();
    while(q.next()){Employe e;e.id=q.value(0).toInt();e.nom=q.value(1).toString();e.prenom=q.value(2).toString();e.age=q.value(3).toInt();e.email=q.value(4).toString();e.telephone=q.value(5).toString();e.poste=q.value(6).toString();e.departement=q.value(7).toString();e.dateEmbauche=q.value(8).toDate();e.idUsine=q.value(9).toInt();employes.append(e);}
    return true;
}

void GestionEmploye::reload(){loadFromDatabase();refreshTable();}

int GestionEmploye::indexById(int id)const{for(int i=0;i<employes.size();i++)if(employes[i].id==id)return i;return -1;}
int GestionEmploye::selectedId()const{int r=table?table->currentRow():-1;return r>=0&&table->item(r,0)?table->item(r,0)->text().toInt():-1;}
int GestionEmploye::nextId()const{int m=1000;for(const auto&e:employes)m=qMax(m,e.id);return m+1;}

void GestionEmploye::refreshTable(){
    table->setSortingEnabled(false);table->clearContents();table->setRowCount(0);QString q=searchEdit?searchEdit->text().trimmed().toLower():"";QString dep=departementFilter?departementFilter->currentText():"Tous";
    for(const Employe&e:employes){QStringList vals={QString::number(e.id),e.nom,e.prenom,QString::number(e.age),e.email,e.telephone,e.poste,e.departement,e.dateEmbauche.toString("dd/MM/yyyy")};bool ok=dep=="Tous"||e.departement==dep;if(!q.isEmpty()){bool hit=false;for(const QString&s:vals)if(s.toLower().contains(q)){hit=true;break;}ok=ok&&hit;}if(!ok)continue;int r=table->rowCount();table->insertRow(r);for(int c=0;c<vals.size();c++)table->setItem(r,c,new QTableWidgetItem(vals[c]));}
    table->resizeColumnsToContents();table->horizontalHeader()->setStretchLastSection(true);updateStatistics();updateCharts();}

void GestionEmploye::updateStatistics(){int n=employes.size(),sumAge=0;for(const auto&e:employes)sumAge+=e.age;if(totalLabel)totalLabel->setText(QString::number(n));if(ageMoyenLabel)ageMoyenLabel->setText(n?QString::number((double)sumAge/n,'f',1)+" ans":"0");}

void GestionEmploye::updateCharts(){
    if(!pieChart||!statsMode)return;
    QVector<double> values;QVector<QString> labels;
    if(statsMode->currentText()=="Âge"){values={0,0,0,0};labels={"18-25","26-35","36-45","46+"};for(const auto&e:employes){if(e.age<=25)values[0]++;else if(e.age<=35)values[1]++;else if(e.age<=45)values[2]++;else values[3]++;}}
    else if(statsMode->currentText()=="Poste"){values=QVector<double>(4,0);labels={"Ingénieur","Technicien","RH","Comptable"};for(const auto&e:employes){int i=postes().indexOf(e.poste);if(i>=0)values[i]++;}}
    else{values=QVector<double>(4,0);labels={"Maintenance","Production","Laboratoire","Usage Technique"};for(const auto&e:employes){int i=departements().indexOf(e.departement);if(i>=0)values[i]++;}}
    pieChart->setData(values,labels);
}

bool GestionEmploye::readForm(Employe&e,bool checkId){
    auto err=[this](const QString&m){QMessageBox::warning(this,"Contrôle de saisie",m);return false;};
    bool ok=false;int v=idEdit->text().trimmed().toInt(&ok);
    if(checkId){if(!ok||v<=0)return err("L'ID doit être un nombre entier positif.");}else v=0;
    static const QRegularExpression nameRx("^[\\p{L}][\\p{L} '\\-]*$");
    static const QRegularExpression telRx("^\\+?\\d{8,15}$");
    const QString nom=nomEdit->text().trimmed(),prenom=prenomEdit->text().trimmed(),email=emailEdit->text().trimmed(),tel=telephoneEdit->text().trimmed();
    if(nom.isEmpty())return err("Le nom est obligatoire.");
    if(nom.size()>60||!nameRx.match(nom).hasMatch())return err("Nom invalide (lettres uniquement, 60 caractères maximum).");
    if(prenom.isEmpty())return err("Le prénom est obligatoire.");
    if(prenom.size()>60||!nameRx.match(prenom).hasMatch())return err("Prénom invalide (lettres uniquement, 60 caractères maximum).");
    if(ageSpin->value()<18||ageSpin->value()>100)return err("L'âge doit être compris entre 18 et 100 ans.");
    if(email.isEmpty())return err("L'email est obligatoire.");
    if(email.size()>100||!validEmail(email))return err("Email invalide (exemple : nom@domaine.tn).");
    if(tel.isEmpty())return err("Le téléphone est obligatoire.");
    QString t=tel;t.remove(' ').remove('-').remove('.');
    if(tel.size()>30||!telRx.match(t).hasMatch())return err("Téléphone invalide (8 à 15 chiffres, exemple : 22 111 111).");
    if(posteCombo->currentText().trimmed().isEmpty())return err("Le poste est obligatoire.");
    if(departementCombo->currentText().trimmed().isEmpty())return err("Le département est obligatoire.");
    if(!dateEdit->date().isValid()||dateEdit->date()>QDate::currentDate())return err("La date d'embauche est invalide (elle ne peut pas être dans le futur).");
    e.id=v;e.nom=nom;e.prenom=prenom;e.age=ageSpin->value();e.email=email;e.telephone=tel;e.poste=posteCombo->currentText();e.departement=departementCombo->currentText();e.dateEmbauche=dateEdit->date();return true;
}

void GestionEmploye::fillForm(const Employe&e){idEdit->setText(QString::number(e.id));nomEdit->setText(e.nom);prenomEdit->setText(e.prenom);ageSpin->setValue(e.age);emailEdit->setText(e.email);telephoneEdit->setText(e.telephone);posteCombo->setCurrentText(e.poste);departementCombo->setCurrentText(e.departement);dateEdit->setDate(e.dateEmbauche);}
void GestionEmploye::clearForm(){if(idEdit)idEdit->clear();if(nomEdit)nomEdit->clear();if(prenomEdit)prenomEdit->clear();if(ageSpin)ageSpin->setValue(18);if(emailEdit)emailEdit->clear();if(telephoneEdit)telephoneEdit->clear();if(posteCombo)posteCombo->setCurrentIndex(0);if(departementCombo)departementCombo->setCurrentIndex(0);if(dateEdit)dateEdit->setDate(QDate::currentDate());if(table)table->clearSelection();}

void GestionEmploye::selectEmploye(){int id=selectedId();int i=indexById(id);if(i>=0)fillForm(employes[i]);}

void GestionEmploye::addEmploye(){
    Connection &c=Connection::createInstance();
    if(!c.ensureOpen()){QMessageBox::critical(this,"Base de données","Connexion Oracle impossible :\n"+c.lastErrorText());return;}
    loadFromDatabase();                                   // données à jour avant les contrôles d'unicité
    const bool autoId=idEdit->text().trimmed().isEmpty();
    Employe e{};
    if(!readForm(e,!autoId))return;
    if(defaultUsineId<=0)loadUsines();
    e.idUsine=defaultUsineId;                             // 0 = aucune usine : ID_USINE sera NULL si la colonne le permet
    if(autoId){                                           // ID automatique : séquence Oracle
        QSqlQuery s(c.database());
        if(!s.exec("SELECT SEQ_EMPLOYE.NEXTVAL FROM DUAL")||!s.next()){QMessageBox::critical(this,"Base de données","Séquence SEQ_EMPLOYE inaccessible (exécutez sql/adaptation_table_employe.sql) :\n"+oracleMessage(s.lastError()));return;}
        e.id=s.value(0).toInt();idEdit->setText(QString::number(e.id));
    }
    if(indexById(e.id)>=0){QMessageBox::warning(this,"Contrôle de saisie","Cet ID existe déjà.");return;}
    for(const Employe &x:employes)if(x.email.compare(e.email,Qt::CaseInsensitive)==0){QMessageBox::warning(this,"Contrôle de saisie","Cet email est déjà utilisé.");return;}
    QSqlQuery q(c.database());
    q.prepare("INSERT INTO EMPLOYE (ID_EMPLOYE,NOM,PRENOM,AGE,EMAIL,TELEPHONE,POSTE,DEPARTEMENT,DATE_EMBAUCHE,ID_USINE) VALUES (?,?,?,?,?,?,?,?,?,?)");
    q.addBindValue(e.id);q.addBindValue(e.nom);q.addBindValue(e.prenom);q.addBindValue(e.age);q.addBindValue(e.email);q.addBindValue(e.telephone);q.addBindValue(e.poste);q.addBindValue(e.departement);q.addBindValue(e.dateEmbauche);q.addBindValue(usineValue(e.idUsine));
    if(!q.exec()){QMessageBox::critical(this,"Ajouter",oracleMessage(q.lastError()));return;}
    reload();QMessageBox::information(this,"Ajouter","Employé ajouté avec succès dans la base Oracle.");clearForm();
}

void GestionEmploye::modifyEmploye(){
    const int oldId=selectedId();
    if(oldId<0){QMessageBox::warning(this,"Modifier","Sélectionnez un employé dans la liste.");return;}
    Connection &c=Connection::createInstance();
    if(!c.ensureOpen()){QMessageBox::critical(this,"Base de données","Connexion Oracle impossible :\n"+c.lastErrorText());return;}
    loadFromDatabase();
    const int i=indexById(oldId);
    if(i<0){QMessageBox::warning(this,"Modifier","Cet employé n'existe plus dans la base.");reload();return;}
    Employe e=employes[i];
    if(!readForm(e,true))return;
    const int other=indexById(e.id);
    if(other>=0&&other!=i){QMessageBox::warning(this,"Contrôle de saisie","Ce nouvel ID existe déjà.");return;}
    for(int j=0;j<employes.size();++j)if(j!=i&&employes[j].email.compare(e.email,Qt::CaseInsensitive)==0){QMessageBox::warning(this,"Contrôle de saisie","Cet email est déjà utilisé.");return;}
    QSqlQuery q(c.database());
    q.prepare("UPDATE EMPLOYE SET ID_EMPLOYE=?,NOM=?,PRENOM=?,AGE=?,EMAIL=?,TELEPHONE=?,POSTE=?,DEPARTEMENT=?,DATE_EMBAUCHE=?,ID_USINE=? WHERE ID_EMPLOYE=?");
    q.addBindValue(e.id);q.addBindValue(e.nom);q.addBindValue(e.prenom);q.addBindValue(e.age);q.addBindValue(e.email);q.addBindValue(e.telephone);q.addBindValue(e.poste);q.addBindValue(e.departement);q.addBindValue(e.dateEmbauche);q.addBindValue(usineValue(e.idUsine));q.addBindValue(oldId);
    if(!q.exec()){QMessageBox::critical(this,"Modifier",oracleMessage(q.lastError()));return;}
    if(q.numRowsAffected()==0){QMessageBox::warning(this,"Modifier","Aucune ligne modifiée : l'employé n'existe plus.");reload();return;}
    reload();QMessageBox::information(this,"Modifier","Employé modifié avec succès dans la base Oracle.");
}

void GestionEmploye::deleteEmploye(){
    const int id=selectedId();
    if(id<0){QMessageBox::warning(this,"Supprimer","Sélectionnez un employé dans la liste.");return;}
    if(QMessageBox::question(this,"Confirmation","Supprimer cet employé de la base Oracle ?",QMessageBox::Yes|QMessageBox::No)!=QMessageBox::Yes)return;
    Connection &c=Connection::createInstance();
    if(!c.ensureOpen()){QMessageBox::critical(this,"Base de données","Connexion Oracle impossible :\n"+c.lastErrorText());return;}
    QSqlQuery q(c.database());
    q.prepare("DELETE FROM EMPLOYE WHERE ID_EMPLOYE=?");q.addBindValue(id);
    if(!q.exec()){QMessageBox::critical(this,"Supprimer",oracleMessage(q.lastError()));return;}
    if(q.numRowsAffected()==0){QMessageBox::warning(this,"Supprimer","Aucune ligne supprimée : l'employé n'existe plus.");reload();return;}
    reload();clearForm();QMessageBox::information(this,"Supprimer","Employé supprimé avec succès de la base Oracle.");
}

void GestionEmploye::searchEmployes(){refreshTable();}
void GestionEmploye::sortEmployes(){reload();}   // tri effectué par Oracle (ORDER BY) dans loadFromDatabase()

void GestionEmploye::exportEmployes(){QString path=QFileDialog::getSaveFileName(this,"Exporter les employés","Employes.csv","Excel CSV (*.csv)");if(path.isEmpty())return;QFile f(path);if(!f.open(QIODevice::WriteOnly|QIODevice::Text)){QMessageBox::critical(this,"Erreur","Impossible d'ouvrir le fichier.");return;}QTextStream out(&f);out<<"ID;Nom;Prenom;Age;Email;Telephone;Poste;Departement;Date d'embauche\n";for(const Employe&e:employes)out<<e.id<<';'<<e.nom<<';'<<e.prenom<<';'<<e.age<<';'<<e.email<<';'<<e.telephone<<';'<<e.poste<<';'<<e.departement<<';'<<e.dateEmbauche.toString("dd/MM/yyyy")<<'\n';f.close();QMessageBox::information(this,"Exportation","Exportation terminée. Le fichier CSV s'ouvre avec Excel.");}

void GestionEmploye::showStatistics(){
    // Le panneau Statistiques est toujours intégré à droite du tableau des employés.
    // Le bouton ne crée aucune fenêtre : il actualise simplement les données affichées.
    if (m_contentStack && m_employeeContent)
        m_contentStack->setCurrentWidget(m_employeeContent);
    updateStatistics();
    updateCharts();
}


QWidget *GestionEmploye::createChatbotPage(){
    QWidget *page = new QWidget;
    QVBoxLayout *root = new QVBoxLayout(page);
    root->setContentsMargins(0,1,0,1);
    root->setSpacing(4);

    QLabel *title = new QLabel("🤖  Chatbot HydroConnect");
    title->setStyleSheet("font-size:16px;font-weight:800;color:#173A60;");
    QLabel *subtitle = new QLabel("Votre assistant local pour vous aider à utiliser HydroConnect.");
    subtitle->setStyleSheet("font-size:12px;color:#687789;");
    root->addWidget(title);
    root->addWidget(subtitle);

    QFrame *panel = new QFrame;
    panel->setObjectName("panel");
    QVBoxLayout *layout = new QVBoxLayout(panel);
    layout->setContentsMargins(9,8,9,8);
    layout->setSpacing(6);
    m_chatHistory = new QTextEdit;
    m_chatHistory->setReadOnly(true);
    m_chatHistory->setMinimumHeight(68);
    m_chatHistory->setMaximumHeight(78);
    m_chatHistory->setStyleSheet("QTextEdit{background:#F7FAFF;border:1px solid #E1EAF7;border-radius:14px;padding:14px;color:#173A60;font-size:14px;}");
    m_chatHistory->append("<b style='color:#168A83'>HydroBot :</b> Bonjour ! Je suis HydroBot, votre assistant HydroConnect. Posez-moi une question sur les employés, les statistiques ou l'utilisation de l'application.");
    layout->addWidget(m_chatHistory,1);

    QHBoxLayout *entry = new QHBoxLayout;
    m_chatInput = new QLineEdit;
    m_chatInput->setPlaceholderText("Écrivez votre question ici...");
    m_chatInput->setMinimumHeight(34);
    QPushButton *send = button("Envoyer ➤","add");
    send->setMinimumHeight(34);
    entry->addWidget(m_chatInput,1);
    entry->addWidget(send);
    layout->addLayout(entry);
    root->addWidget(panel,1);

    QLabel *hint = new QLabel("Exemples : Combien d'employés ? Comment ajouter un employé ? Comment exporter Excel ?");
    hint->setStyleSheet("color:#687789;font-size:11px;");
    root->addWidget(hint);
    connect(send,&QPushButton::clicked,this,&GestionEmploye::sendChatMessage);
    connect(m_chatInput,&QLineEdit::returnPressed,this,&GestionEmploye::sendChatMessage);
    return page;
}

QString GestionEmploye::chatbotReply(const QString &message) const{
    QString m = message.trimmed().toLower();
    if(m.isEmpty()) return "Écrivez une question pour commencer.";
    if(m.contains("bonjour") || m == "salut" || m.contains("hello"))
        return "Bonjour ! Bienvenue dans HydroConnect. Comment puis-je vous aider ?";
    if(m.contains("combien") && (m.contains("employ") || m.contains("personnel")))
        return QString("La liste actuellement chargée contient %1 employé(s). Pour actualiser les données, revenez à la page Employés.").arg(employes.size());
    if(m.contains("employ") && (m.contains("ajout") || m.contains("ajouter")))
        return "Pour ajouter un employé : ouvrez Employés, remplissez le formulaire (nom, prénom, âge, email, téléphone, poste, département et date d'embauche), puis cliquez sur Ajouter. Les champs sont contrôlés avant l'enregistrement dans Oracle.";
    if(m.contains("employ") && (m.contains("modifier") || m.contains("modification")))
        return "Pour modifier un employé : sélectionnez sa ligne dans le tableau, vérifiez les informations affichées dans le formulaire, effectuez vos changements puis cliquez sur Modifier.";
    if(m.contains("employ") && (m.contains("supprimer") || m.contains("suppression")))
        return "Pour supprimer un employé : sélectionnez sa ligne dans le tableau puis cliquez sur Supprimer. Une confirmation est demandée avant la suppression dans Oracle.";
    if(m.contains("recherch") || m.contains("chercher"))
        return "Dans la page Employés, utilisez la barre de recherche. Vous pouvez rechercher par ID, nom, prénom, email, poste ou département.";
    if(m.contains("tri") || m.contains("trier") || m.contains("classer"))
        return "Utilisez la liste Trier au-dessus du tableau pour classer les employés par nom, âge, poste ou département.";
    if(m.contains("excel") || m.contains("export"))
        return "Dans la page Employés, cliquez sur Exporter Excel et choisissez l'emplacement où enregistrer le fichier.";
    if(m.contains("statistique") || m.contains("statistiques"))
        return "HydroConnect présente des statistiques sur les employés. Vous pouvez consulter la répartition par âge, poste et département depuis le panneau Statistiques.";
    if(m.contains("oracle") || m.contains("base de données") || m.contains("database"))
        return "HydroConnect utilise Oracle via la connexion ODBC configurée dans connection.cpp. Vérifiez le nom du DSN, l'utilisateur et le mot de passe si la connexion échoue.";
    if(m.contains("chatbot") || m.contains("hydrobot"))
        return "Je suis HydroBot, un assistant local intégré à HydroConnect. Je réponds à partir de règles et d'informations prévues dans l'application ; je n'utilise pas de service IA externe.";
    if(m.contains("aide") || m.contains("help"))
        return "Je peux vous aider sur : l'ajout, la modification et la suppression d'employés, la recherche, le tri, l'export Excel, les statistiques, Oracle.";
    return "Je n'ai pas encore de réponse précise à cette question. Essayez de demander de l'aide sur les employés, la recherche, le tri, l'export Excel, les statistiques, Oracle.";
}

void GestionEmploye::sendChatMessage(){
    if(!m_chatInput || !m_chatHistory) return;
    const QString message = m_chatInput->text().trimmed();
    if(message.isEmpty()) return;
    m_chatHistory->append("<b style='color:#287CE2'>Vous :</b> " + message.toHtmlEscaped());
    m_chatHistory->append("<b style='color:#168A83'>HydroBot :</b> " + chatbotReply(message).toHtmlEscaped());
    m_chatInput->clear();
    m_chatInput->setFocus();
}
