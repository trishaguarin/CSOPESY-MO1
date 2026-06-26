#include "SubtractCommand.h"
#include "Process.h"
#include <cctype>

static bool isNumericLiteral(const std::string& s)
{
    if (s.empty()) return false;
    for (char c : s)
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    return true;
}

static int resolveOperand(const std::string& op, SymbolTable& st)
{
    if (isNumericLiteral(op))
        return std::stoi(op);
    return st.getVariable(op);
}

void SubtractCommand::execute(Process* process)
{
    SymbolTable& st = process->getSymbolTable();
    int val1 = resolveOperand(operand1, st);
    int val2 = resolveOperand(operand2, st);
    int result = val1 - val2;
    // Clamp to [0, 65535]
    if (result < 0) result = 0;
    if (result > 65535) result = 65535;
    st.setVariable(dest, result);
}
