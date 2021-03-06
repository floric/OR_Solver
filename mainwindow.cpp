#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->calculateButton, SIGNAL(clicked(bool)), this, SLOT(StartCalculation()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::StartCalculation()
{
    if(solver != NULL) {
        delete solver;
    }

    int selectedIndex = ui->directionComboBox->currentIndex();
    bool isFunctionMaximize = true;
    if(selectedIndex == 1) {
        isFunctionMaximize = false;
    }

    solver = new Solver(ui->functionEdit->text(), isFunctionMaximize);

    // add conditions
    QList<QObject*> conditionsWidgets = ui->conditionsGroup->children();
    for(QObject* widget : conditionsWidgets) {
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget);

        if(lineEdit) {
            solver->AddCondition(lineEdit->text());
        }
    }

    // add non negative variables
    QString varStr = ui->varsLineEdit->text().simplified();
    varStr.replace(" ", "");
    QStringList variables = varStr.split(",");
    solver->AddNonNegativeVaribles(variables.toVector());

    mat T(1,1);

    T << 0 << 1 << 2 << 0 << endr
      << 3 << 1 << 0 << 50 << endr
      << 4 << 0 << 1 << 60 << endr
      << 5 << 1 << 0.5 << 100 << endr
      << 6 << 30 << 5 << 1600 << endr
      << 0 << -5000 << -2500 << 0 << endr;

    solver->Solve(T);
}
