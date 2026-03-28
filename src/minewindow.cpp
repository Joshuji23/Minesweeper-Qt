#include "minewindow.h"
#include "ui_minewindow.h"
#include <QMessageBox>
#include <QInputDialog>
#include <QMouseEvent>
#include <QStyle>
#include <QIcon>
#include <QDebug>

// 将秒数转换为 mm:ss 格式
static QString formatTime(int seconds) {
    int minutes = seconds / 60;
    int secs = seconds % 60;
    return QString("%1:%2").arg(minutes, 2, 10, QChar('0')).arg(secs, 2, 10, QChar('0'));
}

MineWindow::MineWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MineWindow),
    m_mineField(new MineField(this)),
    m_timer(new QTimer(this)),
    m_elapsedTime(0),
    m_fieldLayout(nullptr),
    m_settings("YourCompany", "Minesweeper")
{
    ui->setupUi(this);
    setupUI();
    connectSignals();
    onPrimaryLevel();
}

MineWindow::~MineWindow()
{
    delete ui;
}

void MineWindow::setupUI()
{
    setWindowTitle("扫雷");
    setFixedSize(400, 400);

    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    // 左侧垂直布局放置雷数量和时间，右侧放置按钮
    QHBoxLayout *topLayout = new QHBoxLayout();

    // 雷数量 + 时间
    QVBoxLayout *leftInfoLayout = new QVBoxLayout();
    m_mineCountLabel = new QLabel(" 💣: 10", this);
    m_mineCountLabel->setAlignment(Qt::AlignLeft);
    leftInfoLayout->addWidget(m_mineCountLabel);

    m_timeLabel = new QLabel(" ⏱️: 00:00", this);
    m_timeLabel->setAlignment(Qt::AlignLeft);
    leftInfoLayout->addWidget(m_timeLabel);

    topLayout->addLayout(leftInfoLayout);

    // 右侧按钮布局
    QHBoxLayout *rightButtonLayout = new QHBoxLayout();

    m_resetButton = new QPushButton("😀", this);
    m_resetButton->setFixedSize(32, 32);
    m_resetButton->setFocusPolicy(Qt::NoFocus);
    connect(m_resetButton, &QPushButton::clicked, this, &MineWindow::onNewGame);
    rightButtonLayout->addWidget(m_resetButton);

    QPushButton *recordButton = new QPushButton("记录", this);
    recordButton->setFixedSize(60, 32);
    connect(recordButton, &QPushButton::clicked, this, &MineWindow::showBestTimes);
    rightButtonLayout->addWidget(recordButton);

    topLayout->addLayout(rightButtonLayout);
    topLayout->addStretch(); // 让按钮靠右

    mainLayout->addLayout(topLayout);

    // 雷区
    m_fieldLayout = new QGridLayout();
    m_fieldLayout->setSpacing(1);
    mainLayout->addLayout(m_fieldLayout);

    setCentralWidget(centralWidget);

    // 菜单栏
    QMenu *gameMenu = menuBar()->addMenu("游戏");
    gameMenu->addAction("新游戏", this, &MineWindow::onNewGame);
    gameMenu->addSeparator();
    gameMenu->addAction("初级", this, &MineWindow::onPrimaryLevel);
    gameMenu->addAction("中级", this, &MineWindow::onSecondaryLevel);
    gameMenu->addAction("高级", this, &MineWindow::onAdvancedLevel);
    gameMenu->addAction("自定义", this, &MineWindow::onCustomLevel);
    gameMenu->addSeparator();
    gameMenu->addAction("退出", this, &QApplication::quit);

    QMenu *helpMenu = menuBar()->addMenu("帮助");
    helpMenu->addAction("关于", this, [this]() {
        QMessageBox::about(this, "关于扫雷", "扫雷游戏QT项目尝试");
    });
}

void MineWindow::connectSignals()
{
    connect(m_mineField, &MineField::cellUpdated, this, &MineWindow::onCellUpdated);
    connect(m_mineField, &MineField::gameStateChanged, this, &MineWindow::onGameStateChanged);
    connect(m_mineField, &MineField::remainingMinesChanged, this, &MineWindow::onRemainingMinesChanged);
    connect(m_timer, &QTimer::timeout, this, &MineWindow::onTimerTick);
}

void MineWindow::createCellButtons()
{
    // 清除旧按钮
    for (auto &row : m_cellButtons) {
        for (QPushButton *btn : row) {
            if (btn) {
                m_fieldLayout->removeWidget(btn);
                delete btn;
            }
        }
    }
    m_cellButtons.clear();

    int width = m_mineField->getWidth();
    int height = m_mineField->getHeight();

    m_cellButtons.resize(height);
    for (int i = 0; i < height; ++i) {
        m_cellButtons[i].resize(width);
        for (int j = 0; j < width; ++j) {
            QPushButton *btn = new QPushButton(this);
            btn->setFixedSize(24, 24);
            btn->setFocusPolicy(Qt::NoFocus);
            btn->setProperty("row", i);
            btn->setProperty("col", j);

            // 左键：使用 clicked 信号
            connect(btn, &QPushButton::clicked, this, &MineWindow::onCellLeftClicked);

            // 右键：使用 customContextMenuRequested 信号
            btn->setContextMenuPolicy(Qt::CustomContextMenu);
            connect(btn, &QPushButton::customContextMenuRequested, this, &MineWindow::onCellRightClicked);

            // 事件过滤器用于检测左右键同时按下
            btn->installEventFilter(this);

            m_fieldLayout->addWidget(btn, i, j);
            m_cellButtons[i][j] = btn;
        }
    }

    // 调整窗口大小
    int windowWidth = qMax(300, width * 24 + 40);
    int windowHeight = qMax(300, height * 24 + 100);
    setFixedSize(windowWidth, windowHeight);
}

void MineWindow::updateCellButton(int row, int col)
{
    QPushButton *btn = m_cellButtons[row][col];
    if (!btn) return;

    const MineField::Cell &cell = m_mineField->getCell(row, col);
    QString style;
    QString text;

    switch (cell.state) {
    case MineField::State_Normal:
        text = "";
        style = "QPushButton { background-color: #C0C0C0; border: 1px solid #808080; }"
                "QPushButton:hover { background-color: #D0D0D0; }";
        btn->setEnabled(true);
        break;
    case MineField::State_Empty:
        text = "";
        style = "QPushButton { background-color: #E0E0E0; border: 1px solid #808080; }";
        btn->setEnabled(false);
        break;
    case MineField::State_Flag:
        text = "🚩";
        style = "QPushButton { background-color: #C0C0C0; border: 1px solid #808080; }";
        btn->setEnabled(true);
        break;
    case MineField::State_Dicey:
        text = "?";
        style = "QPushButton { background-color: #C0C0C0; border: 1px solid #808080; }";
        btn->setEnabled(true);
        break;
    case MineField::State_Blast:
        text = "💣";
        style = "QPushButton { background-color: #FF4444; border: 1px solid #808080; }";
        btn->setEnabled(false);
        break;
    case MineField::State_Mine:
        text = "💣";
        style = "QPushButton { background-color: #FFCCCC; border: 1px solid #808080; }";
        btn->setEnabled(false);
        break;
    case MineField::State_Error:
        text = "❌";
        style = "QPushButton { background-color: #FF8888; border: 1px solid #808080; }";
        btn->setEnabled(false);
        break;
    default:
        if (cell.state >= 1 && cell.state <= 8) {
            text = QString::number(cell.state);
            // 数字颜色
            QString colors[] = {"", "#0000FF", "#008000", "#FF0000", "#000080",
                                "#800000", "#008080", "#000000", "#808080"};
            QString color = colors[cell.state];
            style = QString("QPushButton { background-color: #E0E0E0; border: 1px solid #808080; "
                            "color: %1; font-weight: bold; }").arg(color);
            btn->setEnabled(false);
        }
        break;
    }

    btn->setText(text);
    btn->setStyleSheet(style);
}

void MineWindow::updateUI()
{
    int width = m_mineField->getWidth();
    int height = m_mineField->getHeight();

    for (int i = 0; i < height; ++i) {
        for (int j = 0; j < width; ++j) {
            updateCellButton(i, j);
        }
    }

    m_mineCountLabel->setText(QString(" 💣: %1").arg(m_mineField->getRemainingMines()));
    m_timeLabel->setText(" ⏱️: " + formatTime(m_elapsedTime));
}

void MineWindow::resetGame()
{
    m_timer->stop();
    m_elapsedTime = 0;
    m_timeLabel->setText(" ⏱️: 00:00");
    m_resetButton->setText("😀");
    createCellButtons();
    updateUI();
}

void MineWindow::saveBestTime(int time)
{
    int width = m_mineField->getWidth();
    int height = m_mineField->getHeight();
    int mines = m_mineField->getMineCount();
    QString key;
    if (width == 9 && height == 9 && mines == 10) key = "primary";
    else if (width == 16 && height == 16 && mines == 40) key = "secondary";
    else if (width == 30 && height == 16 && mines == 99) key = "advanced";
    else return;

    int currentBest = m_settings.value(key, 0).toInt();
    if (currentBest == 0 || time < currentBest) {
        m_settings.setValue(key, time);
    }
}

void MineWindow::onNewGame()
{
    m_mineField->reset();
    resetGame();
}

void MineWindow::onPrimaryLevel()
{
    m_mineField->initialize(9, 9, 10);
    resetGame();
}

void MineWindow::onSecondaryLevel()
{
    m_mineField->initialize(16, 16, 40);
    resetGame();
}

void MineWindow::onAdvancedLevel()
{
    m_mineField->initialize(30, 16, 99);
    resetGame();
}

void MineWindow::onCustomLevel()
{
    bool ok;
    int width = QInputDialog::getInt(this, "自定义游戏", "宽度:", 9, 5, 50, 1, &ok);
    if (!ok) return;
    int height = QInputDialog::getInt(this, "自定义游戏", "高度:", 9, 5, 50, 1, &ok);
    if (!ok) return;
    int mines = QInputDialog::getInt(this, "自定义游戏", "地雷数:", 10, 1, width * height / 2, 1, &ok);
    if (!ok) return;

    m_mineField->initialize(width, height, mines);
    resetGame();
}

void MineWindow::onTimerTick()
{
    if (m_mineField->getGameState() == MineField::GS_Run) {
        m_elapsedTime++;
        m_timeLabel->setText(" ⏱️: " + formatTime(m_elapsedTime));
    }
}

void MineWindow::onCellLeftClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;

    MineField::GameState state = m_mineField->getGameState();
    if (state == MineField::GS_Dead || state == MineField::GS_Victory) {
        return;
    }

    int row = btn->property("row").toInt();
    int col = btn->property("col").toInt();

    if (state == MineField::GS_Wait) {
        m_timer->start(1000);
    }

    m_mineField->revealCell(row, col);
}

void MineWindow::onCellRightClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int row = btn->property("row").toInt();
    int col = btn->property("col").toInt();
    m_mineField->toggleFlag(row, col);
}

bool MineWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        QPushButton *btn = qobject_cast<QPushButton*>(obj);
        if (btn && (mouseEvent->buttons() & Qt::LeftButton) && (mouseEvent->buttons() & Qt::RightButton)) {
            int row = btn->property("row").toInt();
            int col = btn->property("col").toInt();
            m_mineField->openAround(row, col);
            return true;
        }
    }
    return QMainWindow::eventFilter(obj, event);
}

void MineWindow::onCellUpdated(int row, int col)
{
    updateCellButton(row, col);
}

void MineWindow::onGameStateChanged(MineField::GameState state)
{
    switch (state) {
    case MineField::GS_Run:
        m_resetButton->setText("😐");
        break;
    case MineField::GS_Dead:
        m_resetButton->setText("😵");
        m_timer->stop();
        QMessageBox::information(this, "游戏结束", "你输了！");
        break;
    case MineField::GS_Victory:
        m_resetButton->setText("😎");
        m_timer->stop();
        if (m_elapsedTime > 0) {
            saveBestTime(m_elapsedTime);
        }
        QMessageBox::information(this, "游戏结束", QString("恭喜你赢了！用时 %1 秒").arg(m_elapsedTime));
        break;
    default:
        break;
    }
}

void MineWindow::onRemainingMinesChanged(int count)
{
    m_mineCountLabel->setText(QString(" 💣: %1").arg(count));
}

void MineWindow::showBestTimes()
{
    int primaryBest = m_settings.value("primary", 0).toInt();
    int secondaryBest = m_settings.value("secondary", 0).toInt();
    int advancedBest = m_settings.value("advanced", 0).toInt();

    QString text = QString("初级: %1\n中级: %2\n高级: %3")
                       .arg(primaryBest > 0 ? QString::number(primaryBest) + " 秒" : "无记录")
                       .arg(secondaryBest > 0 ? QString::number(secondaryBest) + " 秒" : "无记录")
                       .arg(advancedBest > 0 ? QString::number(advancedBest) + " 秒" : "无记录");

    QMessageBox::information(this, "最佳时间记录", text);
}