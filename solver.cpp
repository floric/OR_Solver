#include "solver.h"

Solver::Solver(QString str, bool toMaximize)
{
    isToMaximize = toMaximize;
    this->mainFunction = ReadFunction(str);
    if(!isToMaximize) {
        cout << "Bringing function to maximize by multiplying all factors with -1!";

        QMapIterator<QString, double> it(mainFunction.variables);
        while (it.hasNext()) {
            it.next();
            mainFunction.variables.insert(it.key(), -1 * it.value());
        }
    }

    cout << "Solver created!" << endl;
}

void Solver::ParseTerm(QString str, Equation* eq)
{
    QRegExp summandSigns("([-+])");
    QRegExp factorSigns("([*])");
    QStringList summands = str.split(summandSigns, QString::SkipEmptyParts);

    for (int i = 0; i < summands.size(); ++i) {
        QString summand = summands.at(i);

        // split factors and variables
        QStringList factors = summand.split(factorSigns, QString::SkipEmptyParts);
        double constant = 1;
        QString varName = "";
        bool castSuccess = false;

        for (int k = 0; k < factors.size(); ++k) {
            QString factor = factors.at(k);

            // find literals and constants
            double val = factor.toDouble(&castSuccess);
            if(castSuccess) {
                constant *= val;
            } else {
                varName += factor;
            }
        }

        if(!eq->variables.contains(varName)) {
            eq->variables.insert(varName, constant);
        } else {
            eq->variables.insert(varName, constant + eq->variables.value(varName));
        }
    }

    eq->isValid = true;
}

Solver::Equation Solver::ReadFunction(QString str)
{
    bool successfullyRead = true;
    Equation eq;

    str = str.simplified();
    str.replace(" ", "");

    QRegExp equationSigns("([<=]|[==]|[>=]|[<]|[>])");

    QMap<QString, double> variables;

    QStringList equationParts = str.split(equationSigns, QString::SkipEmptyParts);

    if(equationParts.size() == 2) {
        // set sign
        if(str.contains("<=")) {
            eq.sign = FunctionSign::LtE;
        } else if(str.contains(">=")) {
            eq.sign = FunctionSign::GtE;
        } else if(str.contains("<")) {
            eq.sign = FunctionSign::Lt;
        } else if(str.contains("==")) {
            eq.sign = FunctionSign::E;
        } else if(str.contains(">")) {
            eq.sign = FunctionSign::Gt;
        } else {
            eq.isValid = false;
        }

        for(int i = 0; i < 2; ++i) {
            QString part = equationParts.at(i);
            bool castSuccess = false;
            double constant = part.toDouble(&castSuccess);

            if(castSuccess) {
                eq.value = constant;
            } else {
                // split summands
                ParseTerm(part, &eq);
            }
        }
    } else if(equationParts.size() == 1) {
        ParseTerm(equationParts.at(0), &eq);
    } else {
        cout << "Error!";
        eq.isValid = false;
    }

    // output read variables
    /*QMapIterator<QString, double> it(eq.variables);
    while (it.hasNext()) {
        it.next();
        cout << "Variable: " << it.key().toStdString() << " with factor: " << it.value() << endl;
    }*/

    return eq;
}

bool Solver::AddCondition(QString str)
{
    Equation eq = ReadFunction(str);

    if(eq.isValid) {
        conditions.append(eq);
    }

    return true;
}

void Solver::AddNonNegativeVaribles(QVector<QString> variables)
{
    for(QString varName : variables) {
        nonNegativeVars.insert(varName);
    }
}

QString Solver::GetConditionsAsString()
{
    QString str;
    for(Equation eq: conditions) {
        str += GetEquationAsString(eq);
    }

    return str;
}

QString Solver::GetEquationAsString(Solver::Equation eq)
{
    QString str;

    if(eq.isValid) {
        QMapIterator<QString, double> it(eq.variables);
        while (it.hasNext()) {
            it.next();
            str += QString::number(it.value()) + " * " + it.key();

            if(it.hasNext()) {
                str += " + ";
            } else {
                str += SignToString(eq.sign) + QString::number(eq.value) + "\n";
            }
        }


    } else {
        str.append("Equation not valid!");
    }

    return str;
}

void Solver::AddSlackVars()
{

    // add slack vars for every non negative variable
    for(Equation eq : conditions) {
        QMapIterator<QString, double> it(eq.variables);
        while(it.hasNext()) {
            it.next();
            usedVars.insert(it.key());
        }
    }

    QMapIterator<QString, double> itFunVars(mainFunction.variables);
    while(itFunVars.hasNext()) {
        itFunVars.next();
        usedVars.insert(itFunVars.key());
    }


    // set beginning index of slack variables
    QSetIterator<QString> itUsedVars(usedVars);
    while(itUsedVars.hasNext()) {
        int varIndex = itUsedVars.next().at(1).unicode() - 48;
        if(varIndex > slackIndex) {
            slackIndex = varIndex + 1;
        }
    }

    QSetIterator<QString> it((QSet<QString>(usedVars)).subtract(nonNegativeVars));
    while(it.hasNext()) {
        const QString oldVar = it.next();
        const QString newSlackVar = GetNewSlackVariable();
        slackVars.insert(oldVar, newSlackVar);
        mainFunction.variables.insert(newSlackVar, mainFunction.variables.value(oldVar) * -1);
    }

    QMapIterator<QString, QString> mapIt(slackVars);
    while(mapIt.hasNext()) {
        mapIt.next();
        cout << "New slack variable: " << mapIt.key().toStdString() << " gets " << mapIt.value().toStdString() << endl;
    }

    // add slack vars for every inequation
    for(int i = 0; i < conditions.size(); ++i) {
        Equation eq = conditions.at(i);
        if(eq.sign == Solver::FunctionSign::LtE) {
            eq.variables.insert(GetNewSlackVariable(), 1);
            eq.sign = Solver::FunctionSign::E;
        }

        conditions[i] = eq;
    }
}

mat Solver::Solve(mat M)
{


    // bring function normal form


    // make all variables >= 0 and add vars
    AddSlackVars();
    cout << "Function:\n" << GetEquationAsString(mainFunction).toStdString() << endl;
    cout << "Conditions:\n" << GetConditionsAsString().toStdString() << endl;

    /*cout << "Solve for M(Rows=" << M.n_rows << "; Cols=" << M.n_cols << "):" << endl;
    M.print("M:");

    // Apply simplex algorithm
    int iterations = 0;

    while(iterations < 10) {
        int l = FindPivotColumn(M);

        if(l == -1) {
            cout << "Maximum found: " << M.at(M.n_rows - 1, M.n_cols - 1) << endl;
            break;
        }

        int k = FindPivotRow(M, l);
        if(k == -1) {
            M.print("Function not bordered:");
            break;
        }

        iterations++;
        cout << "Iteration: " << iterations << endl;
        cout << "Use pivot: l=" << l << "; k=" << k << endl;

        CalcNewTable(&M, l, k);

        // swap variable indizes
        double temp = M.at(k, 0);
        M.at(k, 0) = M.at(0, l);
        M.at(0, l) = temp;

        M.print("After iteration:");
    }*/

    return M;
}

int Solver::FindPivotColumn(const mat &M)
{
    int pivotColumnIndex = -1;

    double lowestValue = 9999999;
    for(int l = 1; l < (M.n_cols - 1); ++l) {
        double gl = M.at(M.n_rows - 1, l);

        if(gl < 0) {
            if(gl < lowestValue) {
                lowestValue = gl;
                pivotColumnIndex = l;
            }
        }
    }

    return pivotColumnIndex;
}

int Solver::FindPivotRow(const mat &M, int l)
{
    int pivotRowIndex = -1;

    double lowestValue = 9999999;
    double currentValue = 9999999;

    for(int k = 1; k < (M.n_rows - 1); ++k) {
        double sk = M.at(k, M.n_cols - 1);
        double rkl = M.at(k, l);

        if(rkl > 0.001) {
            currentValue = sk / rkl;

            if(currentValue < lowestValue) {
                lowestValue = currentValue;
                pivotRowIndex = k;
            }
        }
    }

    return pivotRowIndex;
}

void Solver::CalcNewTable(mat *M, int lPiv, int kPiv)
{
    mat newM(*M);

    // calculate new pivot element
    double rkl = M->at(kPiv, lPiv);

    // new pivot element
    newM.at(kPiv, lPiv) = 1 / rkl;

    // calculate new pivot row
    for(int l = 1; l < M->n_cols; ++l) {
        if(l != lPiv) {
            newM.at(kPiv, l) = M->at(kPiv, l) / rkl;
        }
    }

    // calculate new pivot column
    for(int k = 1; k < M->n_rows; ++k) {
        if(k != kPiv) {
            newM.at(k, lPiv) = (-1) * (M->at(k, lPiv) / rkl);
        }
    }

    // calculate other elements
    for(int i = 1; i < M->n_rows; ++i) {
        for(int j = 1; j < M->n_cols; ++j) {
            if(i != kPiv && j != lPiv) {
                newM.at(i, j) = M->at(i, j) - ((M->at(i, lPiv) * M->at(kPiv, j)) / rkl);
            }
        }
    }

    // calculate new w

    *M = newM;
}

bool Solver::IsOptimalSolution(const mat &M)
{
    for (int l = 0; l < M.n_cols; ++l) {
        if(M.at(M.n_rows - 1, l) < 0.1) {
            return false;
        }
    }

    return true;
}

QString Solver::SignToString(Solver::FunctionSign sign)
{
    switch(sign) {
        case FunctionSign::Lt:
            return "<";
        break;
        case FunctionSign::LtE:
            return "<=";
        break;
        case FunctionSign::E:
            return "=";
        break;
        case FunctionSign::Gt:
            return ">";
        break;
        case FunctionSign::GtE:
            return ">=";
        break;
        default:
            return "?";
    }
}

QString Solver::GetNewSlackVariable()
{
    return ("x" + QString::number(slackIndex++));
}

