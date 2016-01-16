#ifndef SOLVER_H
#define SOLVER_H

#include <armadillo>

#include <QString>
#include <QStringList>
#include <QObject>
#include <QMap>
#include <QVector>
#include <QSet>

using namespace arma;

class Solver : public QObject
{
    Q_OBJECT
    Q_ENUMS(FunctionSign)

public:
    enum FunctionSign { Lt, LtE, E, GtE, Gt };

    struct Equation {
        QMap<QString, double> variables;
        double value = 0.0;
        FunctionSign sign = E;
        bool isValid = false;
    };

    Solver(QString str, bool toMaximize);

    Equation ReadFunction(QString str);
    bool AddCondition(QString str);
    void AddNonNegativeVaribles(QVector<QString> variables);

    QString GetConditionsAsString();

    mat Solve(mat M);

    void ParseTerm(QString str, Equation *eq);

    void AddSlackVars();
private:
    Equation mainFunction;
    bool isToMaximize = true;

    QVector<Equation> conditions;
    QSet<QString> nonNegativeVars;
    QSet<QString> usedVars;
    QMap<QString, QString> slackVars;

    int FindPivotColumn(const mat& M);
    int FindPivotRow(const mat& M, int l);
    void CalcNewTable(mat* M, int lPiv, int kPiv);

    bool IsOptimalSolution(const mat& M);
    QString SignToString(FunctionSign sign);

    QString GetNewSlackVariable();

    int slackIndex = 0;
};

#endif // SOLVER_H
