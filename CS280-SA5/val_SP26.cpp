#include "val_SP26.h"
#include <cctype>
#include <algorithm>

Value Value::operator+(const Value& op) const {
	// Numeric addition (int + real mixed allowed)
	if ((IsInt() || IsReal()) && (op.IsInt() || op.IsReal())) {
		if (IsReal() || op.IsReal()) {
			double lhs = IsReal() ? Rtemp : Itemp;
			double rhs = op.IsReal() ? op.Rtemp : op.Itemp;
			return Value(lhs + rhs);
		}
		return Value(Itemp + op.Itemp);
	}

	// String / char concatenation
	if ((IsString() || IsChar()) && (op.IsString() || op.IsChar())) {
		string lhs = IsString() ? Stemp : string(1, Ctemp);
		string rhs = op.IsString() ? op.Stemp : string(1, op.Ctemp);
		return Value(lhs + rhs);
	}

	return Value(); // VERR
}


Value Value::operator/(const Value& op) const {
	if ((IsInt() || IsReal()) && (op.IsInt() || op.IsReal())) {
		double lhs = IsReal() ? Rtemp : Itemp;
		double rhs = op.IsReal() ? op.Rtemp : op.Itemp;

		if (rhs == 0.0)
			return Value(); // error

		return Value(lhs / rhs); // ALWAYS real
	}

	return Value(); // VERR
}

Value Value::operator<(const Value& op) const {
	// Numeric comparison
	if ((IsInt() || IsReal()) && (op.IsInt() || op.IsReal())) {
		double lhs = IsReal() ? Rtemp : Itemp;
		double rhs = op.IsReal() ? op.Rtemp : op.Itemp;
		return Value(lhs < rhs);
	}
    
    // Bool comparison
    if (IsBool() && op.IsBool()) {
        return Value(Btemp < op.Btemp);
    }

	// String comparison
	if (IsString() && op.IsString()) {
		return Value(Stemp < op.Stemp);
	}

	// Char comparison
	if (IsChar() && op.IsChar()) {
		return Value(Ctemp < op.Ctemp);
	}

	return Value(); // VERR
}


Value Value::Trim() const {
    // Only strings are allowed
    if (!IsString())
        return Value(); // VERR

    string s = Stemp;

    size_t end = s.find_last_not_of(' ');
    if (end == string::npos)
        return Value(string("")); // all spaces → empty string

    return Value(s.substr(0, end + 1));
}