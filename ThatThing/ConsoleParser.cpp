#include "ConsoleParser.h"
#include <exception>
#include <cctype>
#include <algorithm>

std::vector<std::string> LexicalParser::Lexema::AllowedMarks = {
"=", "==",
"<", ">", "<=", ">=",
".", ",",
"(", ")",
"[", "]",
"{", "}",
"\\", "%",
":", "::",
"&", "&&", "|", "||", "!",
"+", "-", "*", "/",
"++", "--",
"+=", "-=", "*=", "/=",
"&=", "|=",
"!", "?"
};

void LexicalParser::PushTmpLexema(RetType& TmpRet, Lexema& TmpLexema)
{
	if (TmpLexema.Type != Lexema::NONE)
	{
		if (TmpLexema.str.size() != 0)
		{
			TmpRet.back().push_back(TmpLexema);
			TmpLexema.str.clear();
		}
		TmpLexema.Type = Lexema::NONE;
	}
}

void LexicalParser::ParseOneCharacter(char & c, RetType& TmpRet, Lexema& TmpLexema)
{
	if (TmpLexema.Type == Lexema::LITERAL)
	{
		if (c == '"') { PushTmpLexema(TmpRet, TmpLexema); return; }
	}
	else if (isdigit(c))
	{
		if (TmpLexema.Type == Lexema::NAME) {/*Then it's still a name. Do nothing*/ }
		else if ((TmpLexema.Type == Lexema::NUMBER) || (TmpLexema.Type == Lexema::NUMBER_DOTTED)) {/*Then it's still a number*/ }
		else if (TmpLexema.str == "-") {
			if ((TmpRet.size() == 0) || (TmpRet.back().back().Type == Lexema::MARK))
			{/*Then this minus is from a number. So just do nothing and let the rest of the number be added to it*/
			}
			else {/*Then this minus is from other lexema. Push it and start a new one*/
				PushTmpLexema(TmpRet, TmpLexema);
			}
			TmpLexema.Type = Lexema::NUMBER;
		}
		else
		{
			PushTmpLexema(TmpRet, TmpLexema);
			TmpLexema.Type = Lexema::NUMBER;
		}
	}
	else if (c == '.')
	{
		if (TmpLexema.Type == Lexema::NUMBER) { TmpLexema.Type = Lexema::NUMBER_DOTTED; }
		else if (TmpLexema.Type == Lexema::NUMBER_DOTTED) { throw std::exception("A Number have more then one dot!"); }
		else
		{
			PushTmpLexema(TmpRet, TmpLexema);
			TmpLexema.Type = Lexema::MARK;
		}
	}
	else if ((c != '_') && (ispunct(c)))
	{
		if ((c == '-') && (TmpLexema.str == "-") && (TmpRet.back().back().Type == Lexema::NUMBER))
		{//Numbers cannot be decremented. So it's definetly just a minus;
			PushTmpLexema(TmpRet, TmpLexema);
			TmpLexema.Type = Lexema::MARK;
		}
		else if (TmpLexema.Type != Lexema::MARK) { PushTmpLexema(TmpRet, TmpLexema);	TmpLexema.Type = Lexema::MARK; }

		if (std::find(Lexema::AllowedMarks.begin(), Lexema::AllowedMarks.end(), (TmpLexema.str + c)) == Lexema::AllowedMarks.end())
		{//Then there is no lexema, that could be fromed adding this character
			if (std::find(Lexema::AllowedMarks.begin(), Lexema::AllowedMarks.end(), TmpLexema.str) == Lexema::AllowedMarks.end())
			{
				throw std::exception("A mark is provided, but this particular is not allowed:" + c);
			}
			PushTmpLexema(TmpRet, TmpLexema);
			TmpLexema.Type = Lexema::MARK;
		}
	}
	else//Then it's a NAME!
	{
		if (TmpLexema.Type != Lexema::NAME) { PushTmpLexema(TmpRet, TmpLexema); TmpLexema.Type = Lexema::NAME; }
	}
	TmpLexema.str += c;
}

std::string LexicalParser::ParseOneCommand(std::string &line, RetType& TmpRet, Lexema& TmpLexema)
{
	TmpRet.emplace_back();//Add new command
	std::string RemainingCommand;

	for (int i = 0; i < line.size(); i++)
	{
		if (TmpLexema.Type == Lexema::LITERAL){ParseOneCharacter(line[i], TmpRet, TmpLexema);}
		else if (isspace(line[i]))	{ PushTmpLexema(TmpRet, TmpLexema); }
		else if (line[i] == '"')	{ PushTmpLexema(TmpRet, TmpLexema); TmpLexema.Type = Lexema::LITERAL;}
		else if (line[i] == ';') { PushTmpLexema(TmpRet, TmpLexema); RemainingCommand = line.substr(i + 1); break; }
		else						{ParseOneCharacter(line[i], TmpRet, TmpLexema);}
	}
	PushTmpLexema(TmpRet, TmpLexema);
	return RemainingCommand;
}


std::vector<std::vector<LexicalParser::Lexema>> LexicalParser::Parse(std::string line)
{
	std::vector<std::vector<Lexema>> RetVec;
	Lexema tmpLexema;
	while (line.size() != 0)
	{
		line = std::move(ParseOneCommand(line, RetVec, tmpLexema));
		if (RetVec.back().size() == 0) { RetVec.erase(RetVec.end()-1); }
	}

	/*
	for (auto& Command : m_parsedLexemas)
	{
		printf("|");
		for (auto& Liter : Command)
		{
			switch (Liter.Type)
			{
			case Lexema::NONE: { printf("<NONE>"); break; }
			case Lexema::NAME: { printf("<NAME>"); break; }
			case Lexema::MARK: { printf("<MARK>"); break; }
			case Lexema::NUMBER: { printf("<NUM>"); break; }
			case Lexema::NUMBER_DOTTED: { printf("<DOT>"); break; }
			default:
				break;
			}
			printf("%s|", Liter.str.c_str());
		}
		printf("\n");
	}
	*/
	return RetVec;
}

std::pair<std::optional<LogicalParser::VariableName>, int> LogicalParser::IsThereVariableName(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	VariableName PotentialObject;
	int Level = 0;
	for (; From < Lexemas.size(); From++)
	{
		//[L1][.][L2][L3] In that case inly L1.L2 will be a single VarName.
		if (Lexemas[From].Type == LexicalParser::Lexema::NAME)	{
			if (PotentialObject.Hierarchy.size() != Level) { break; }
			PotentialObject.Hierarchy.push_back(Lexemas[From].str);
		}

		else if (Lexemas[From].str == ".")	{Level++;} //if there is a dot, then we're going one lever up the hierarchy
		else { break; }//If it niether a dot nor a NAME, then Lexema have ended
	}
	if (PotentialObject.Hierarchy.size() == 0) { return std::make_pair(std::nullopt, -1); }
	//If there is something in hierarchy, then there is a VarName!
	return std::make_pair(PotentialObject, --From);
}

std::optional<LogicalParser::NumericalValue> LogicalParser::IsThereNumerical(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	NumericalValue PotentialObject;
	if (Lexemas[From].Type == LexicalParser::Lexema::NUMBER) {
		PotentialObject.Value = Lexemas[From].str; 
		PotentialObject.isDotted = false;
		return PotentialObject;
	}
	else if (Lexemas[From].Type == LexicalParser::Lexema::NUMBER_DOTTED) {
		PotentialObject.Value = Lexemas[From].str;
		PotentialObject.isDotted = true;
		return PotentialObject;
	}

	return std::nullopt;
}

std::optional<LogicalParser::Literal> LogicalParser::IsThereLiteral(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	Literal PotentialObject;
	if (Lexemas[From].Type == LexicalParser::Lexema::LITERAL)
	{
		PotentialObject.LiteralValue = Lexemas[From].str;
		return PotentialObject;
	}
	return std::nullopt;
}

std::optional<LogicalParser::Operator> LogicalParser::IsThereOperator(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	Operator PotentialObject;
	if (Lexemas[From].Type == LexicalParser::Lexema::MARK)
	{
		PotentialObject.Op = Lexemas[From].str;
		return PotentialObject;
	}
	return std::nullopt;
}

std::list<LogicalParser::LogicalUnit> LogicalParser::ParseCommand(std::vector<LexicalParser::Lexema>& Lexemas)
{
	std::list<LogicalUnit> RetList;
	for (int i = 0; i < Lexemas.size(); i++)
	{
		//First check for names;
		auto NameResult = IsThereVariableName(Lexemas, i);
		if (NameResult.first)//Then we have a name!
		{
			i = NameResult.second;//Shilft iterator to where name ends;
			RetList.emplace_back(LogicalUnit(NameResult.first.value(), UnitType::VAR));
			continue;
		}
		
		auto NumericResult = IsThereNumerical(Lexemas, i);
		if (NumericResult) { RetList.emplace_back(LogicalUnit(NumericResult.value(), UnitType::NUM)); continue; }

		auto OperatorResult = IsThereOperator(Lexemas, i);
		if (OperatorResult) { RetList.emplace_back(LogicalUnit(OperatorResult.value(), UnitType::OP)); continue; }

		auto LiteralResult = IsThereLiteral(Lexemas, i);
		if (LiteralResult) { RetList.emplace_back(LogicalUnit(LiteralResult.value(), UnitType::LIETARL)); continue; }
		
		throw std::exception("A line contains nothing to be logical parsed!");
	}
	return RetList;
}

void LogicalParser::CollapseFunctions(std::list<LogicalUnit>& Units)
{
	for (auto& it = Units.begin(); it != Units.end(); it++)
	{
		if (it->second == UnitType::VAR)
		{
			auto VarIt = it;
			it++;
			if (it == Units.end()) { break; }
			if ((it->second == UnitType::OP) && (std::get<Operator>(it->first).Op == "("))
			{
				Function Func;
				Func.Name = std::get<VariableName>(VarIt->first);
				auto NewIt = Units.erase(VarIt, ++it);
				it = Units.insert(NewIt, LogicalUnit(Func, UnitType::FUNC));
			}
		}
	}
}

std::list<LogicalParser::LogicalUnit> LogicalParser::ToPostfix(std::list<LogicalUnit>& Units)
{
	std::vector<LogicalUnit> Stack;
	std::list<LogicalUnit> Result;

	auto	IAITF_ButOnlyIfItsZero = [&]() {
		for (auto& rit = Stack.rbegin(); rit != Stack.rend(); rit++)
		{
			if (rit->second == UnitType::FUNC) {
				if (std::get<Function>(rit->first).Arity == 0) {
					std::get<Function>(rit->first).Arity++;
				} 
				return;
			}
		}
	};
	auto PushOutOfStuckUntillFunctionOrParentheses = [&]() {
		
		while (Stack.size() != 0)
		{
			if (Stack.back().second == UnitType::FUNC) { Result.push_back(Stack.back()); Stack.pop_back(); return; }
			if ((Stack.back().second == UnitType::OP)&&(std::get<Operator>(Stack.back().first).Op == "("))
			{Stack.pop_back(); return; }
			Result.push_back(Stack.back()); Stack.pop_back();
		}
		//If we're still here
		throw std::exception("Too much ')'");
	};
	auto CheckOperatorOrderAndPutItDown = [&](Operator& Op, LogicalUnit& val)	{
		while (Stack.size() > 0)
		{
			if ((Stack.back().second == UnitType::OP) && (std::get<Operator>(Stack.back().first).getPriority() > Op.getPriority()))
			{
				if(Op.getPriority() == 0){break;}
				Result.push_back(Stack.back()); Stack.pop_back();
			}
			else { break; }
		}
		Stack.push_back(val);
	};
	auto ProcessComma = [&]() {
		while(Stack.size() != 0)
		{
			if (Stack.back().second == UnitType::FUNC) {
				std::get<Function>(Stack.back().first).Arity++; return;
			}
			Result.push_back(Stack.back()); Stack.pop_back();
		}
		throw std::exception("There is something wrong with commas!");
	};

	for (auto& unit : Units)
	{
		switch (unit.second)
		{
			case UnitType::VAR:
			case UnitType::NUM:
			case UnitType::LIETARL: {
				Result.push_back(unit);
				IAITF_ButOnlyIfItsZero();
				break;
			}
			case UnitType::FUNC: {
				IAITF_ButOnlyIfItsZero();
				Stack.push_back(unit);
				break;
			}
			case UnitType::OP: {
				auto& OpRep = std::get<Operator>(unit.first);
				if (OpRep.Op == ",") { ProcessComma(); }
				else if (OpRep.Op == ")") { PushOutOfStuckUntillFunctionOrParentheses();}
				else if (OpRep.isActionOperator()) { CheckOperatorOrderAndPutItDown(OpRep, unit); }
				else {throw std::exception("Unexpected operator!"); }
				break;
			}
		}
	}

	while (Stack.size() != 0)
	{
		if ((Stack.back().second == UnitType::OP) && (std::get<Operator>(Stack.back().first).Op == "("))
		{
			throw std::exception("Too many '('");
		}
		Result.push_back(Stack.back()); Stack.pop_back();
	}
	
	return Result;
}

std::vector<std::list<LogicalParser::LogicalUnit>> LogicalParser::Parse(std::vector<std::vector<LexicalParser::Lexema>> & Lexemics)
{
	std::vector<std::list<LogicalParser::LogicalUnit>> Units;
	for (auto LexList : Lexemics)
	{
		auto& List = ParseCommand(LexList);
		CollapseFunctions(List);
		Units.push_back(ToPostfix(List));
	}
	return Units;
}

void LogicalParser::Print(std::vector<std::list<LogicalParser::LogicalUnit>>& Input)
{
	for (auto& Command : Input)
	{
		printf("|");
		for (auto& Unit : Command)
		{
			switch (Unit.second)
			{
			case UnitType::VAR: {
				printf("<VAR>");
				auto& Hier = std::get<VariableName>(Unit.first).Hierarchy;
				std::string name = Hier[0];
				for (int i = 1; i < Hier.size(); i++) { name += "." + Hier[i]; }
				printf("%s|", name.c_str());
				break;
			}
			case UnitType::LIETARL: {
				printf("<LIT>");
				printf("%s|", std::get<Literal>(Unit.first).LiteralValue.c_str());			
				break;
			}
			case UnitType::NUM: {
				printf("<NUM>");
				printf("%s|", std::get<NumericalValue>(Unit.first).Value.c_str());
				break;
			}
			case UnitType::OP: {
				printf("<OPR>");
				printf("%s|", std::get<Operator>(Unit.first).Op.c_str());
				break;
			}
			case UnitType::FUNC: {
				printf("<FUNC>");
				auto& Hier = std::get<Function>(Unit.first).Name.Hierarchy;
				std::string name = Hier[0];
				for (int i = 1; i < Hier.size(); i++) { name += "." + Hier[i]; }
				printf("%s(%i)|", name.c_str(), std::get<Function>(Unit.first).Arity);
				break;
			}
			default:
				throw("Logical unit misses it's type!");
			}
		}
		printf("\n");
	}
}

int LogicalParser::Operator::getPriority() const
{
	if (Op == "*" || Op == "/") { return 2; }
	if (Op == "+" || Op == "-") { return 1; }
	return 0;
}

bool LogicalParser::Operator::isActionOperator() const
{
	static std::vector<std::string> ActionOps{
		"+", "-", "*", "/",
		"=",
		"==", "<", ">", "<=", ">=",
		"(", "%"
	};

	return std::find(ActionOps.begin(), ActionOps.end(), Op) != ActionOps.end();
}


std::optional<std::map<std::string, Evaluator::Holder>::iterator> Evaluator::FindName(const std::vector<std::string>& Name)
{
	auto FindResult = Sub.find(Name[0]);
	if (FindResult == Sub.end()) { return std::nullopt; }
	for (int i = 1; i < Name.size(); i++)
	{
		auto End = FindResult->second.Sub.end();
		FindResult = FindResult->second.Sub.find(Name[i]);
		if (FindResult == End) { return std::nullopt; }
	}
	return FindResult;
}

void Evaluator::ParseCommand(std::list<LogicalParser::LogicalUnit>& List)
{
	std::vector<LogicalParser::LogicalUnit> Buffer;

	//Finds object with provided name. Or function.
	auto FindObj = [&](LogicalParser::LogicalUnit& Obj) -> Holder& {
		if (Obj.second == LogicalParser::UnitType::VAR)
		{
			auto Result = FindName(std::get<LogicalParser::VariableName>(Obj.first).Hierarchy);
			if(!Result) { throw std::exception("There is no such variable!"); }
			if ((*Result)->second.isType<Function>()) { throw std::exception("Function cannot be used as a variable!"); }
			return (*Result)->second;
		}
		std::exception("Object is used as a variable, but it's not a variable!");
	};

	auto FindFunc = [&](LogicalParser::Function& Func) -> Holder& {

		auto Result = FindName(Func.Name.Hierarchy);
		if (!Result) { throw std::exception("There is no such function!"); }
		return (*Result)->second;
	};
	//Returns true for Numerics and And Numeric/DotNumeric variables. false otherwise
	auto CanBeNUM = [&](LogicalParser::LogicalUnit& Obj) {
		if (Obj.second == LogicalParser::UnitType::NUM) { return true; }
		if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
				if (Found.get<Variable>().Type == OBJTYPE::NUM_DOTTED || Found.get<Variable>().Type == OBJTYPE::NUM)
				{
					return true;
				}
		}
		return false;
	};
	//Returns a string Rep and isDotted;
	auto GetAsNUM = [&](LogicalParser::LogicalUnit& Obj) {

		if (Obj.second == LogicalParser::UnitType::NUM) {
			auto& Sh = std::get<LogicalParser::NumericalValue>(Obj.first);
			return std::make_pair(Sh.Value, Sh.isDotted);
		}
		if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
				if (Found.get<Variable>().Type == OBJTYPE::NUM_DOTTED) { return std::make_pair(Found.get<Variable>().get(), true); }
				else if (Found.get<Variable>().Type == OBJTYPE::NUM) { return std::make_pair(Found.get<Variable>().get(), true); }

		}
		throw std::exception("A value is requested as numeric. But it's not numeric!");
	};

	auto CanBeLiteral = [&](LogicalParser::LogicalUnit& Obj) {
		if (Obj.second == LogicalParser::UnitType::LIETARL) { return true; }
		if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
				if (Found.get<Variable>().Type == OBJTYPE::STR)
				{
					return true;
				}
		}
		return false;
	};
	auto GetAsLiteral = [&](LogicalParser::LogicalUnit& Obj) {

		if (Obj.second == LogicalParser::UnitType::LIETARL) {
			return std::get<LogicalParser::Literal>(Obj.first).LiteralValue;
		}
		if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
			{
				if (Found.get<Variable>().Type == OBJTYPE::STR) { return Found.get<Variable>().get(); }
			}
		}
		throw std::exception("A value is requested as literal. But it's not literal!");
	};
	//Check if it's a variable. If it is, returns it's type
	auto CanBeVariable = [&](LogicalParser::LogicalUnit& Obj) {
		if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
			{
				return std::optional<OBJTYPE>(Found.get<Variable>().Type);
			}
		}
		return std::optional<OBJTYPE>(std::nullopt);
	};
	auto JustGet = [&](LogicalParser::LogicalUnit& Obj) {
		if (Obj.second == LogicalParser::UnitType::NUM) {
			return std::get<LogicalParser::NumericalValue>(Obj.first).Value;
		}
		else if (Obj.second == LogicalParser::UnitType::LIETARL) {
			return std::get<LogicalParser::Literal>(Obj.first).LiteralValue;
		}
		else if (Obj.second == LogicalParser::UnitType::VAR) {
			auto& Found = FindObj(Obj);
			if (Found.isType<Variable>())
			{
				return Found.get<Variable>().get();
			}
		}
	};

	auto ProcessOperator = [&](std::string& op) {
		if (op == "+") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.isDotted = true;
					Numval.Value = std::to_string(std::stof(Left.first) + std::stof(Right.first));
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) + std::stoi(Right.first)); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
			else if (CanBeLiteral(Buffer[Buffer.size() - 1]) && CanBeLiteral(Buffer[Buffer.size() - 2])) {
				LogicalParser::Literal LitVal;
				auto& Left = GetAsLiteral(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsLiteral(Buffer[Buffer.size() - 1]);
				LitVal.LiteralValue = Left + Right;
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(LitVal, LogicalParser::UnitType::LIETARL));
			}
			else { throw std::exception("There is no way to perform + on this arguments!"); }
		}
		else if (op == "-") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.isDotted = true;
					Numval.Value = std::to_string(std::stof(Left.first) - std::stof(Right.first));
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) - std::stoi(Right.first)); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == "*") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.isDotted = true;
					Numval.Value = std::to_string(std::stof(Left.first) * std::stof(Right.first));
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) * std::stoi(Right.first)); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == "/") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.isDotted = true;
					Numval.Value = std::to_string(std::stof(Left.first) / std::stof(Right.first));
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) / std::stoi(Right.first)); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == "%") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					throw std::exception("Operator % cannot be called for dotted numerics!");
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) % std::stoi(Right.first)); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}

		else if (op == "<") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) < std::stof(Right.first) ? 1 : 0);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) < std::stoi(Right.first) ? 1 : 0); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == ">") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) > std::stof(Right.first) ? 1 : 0);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) > std::stoi(Right.first) ? 1 : 0); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == "==") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) == std::stof(Right.first) ? 1 : 0);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) == std::stoi(Right.first) ? 1 : 0); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == "<=") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) <= std::stof(Right.first) ? 1 : 0);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) <= std::stoi(Right.first) ? 1 : 0); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}
		else if (op == ">=") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) >= std::stof(Right.first) ? 1 : 0);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) >= std::stoi(Right.first) ? 1 : 0); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM));
			}
		}

		else if (op == "=") {
			if (CanBeVariable(Buffer[Buffer.size() - 2])) {
				auto& Found = FindObj(Buffer[Buffer.size() - 2]);
				if (CanBeLiteral(Buffer[Buffer.size() - 1]) && Found.get<Variable>().Type == OBJTYPE::STR)
				{
					Found.get<Variable>().set(GetAsLiteral(Buffer[Buffer.size() - 1]));
				}
				else if (CanBeNUM(Buffer[Buffer.size() - 1]))
				{
					auto& Val = GetAsNUM(Buffer[Buffer.size() - 1]);
					if (Found.get<Variable>().Type == OBJTYPE::NUM_DOTTED)
					{
						Found.get<Variable>().set(std::to_string(std::stof(Val.first)));
					}
					else if (!Val.second)
					{
						Found.get<Variable>().set(std::to_string(std::stoi(Val.first)));
					}
					else {
						throw std::exception("Dotted value cannot be assigned to a dotted one!");
					}
				}
			}
			Buffer.pop_back();
		}
		else { throw std::exception("Requested operator cannot be executed. Probably becouse of type missmatch"); }
	};

	auto ProcessFunction = [&](LogicalParser::Function& Func) {
		auto& FuncObj = FindFunc(Func);
		if (FuncObj.isType<Variable>() && Func.Arity != 0) { throw std::exception("Variable functions can't accept parameters!"); }
		else if (FuncObj.isType<Function>())
		{
			auto& FuncFunc = FuncObj.get<Function>();
			if (FuncFunc.Arity < Func.Arity) { throw std::exception("Function can't accept that much parameters!"); }
			std::vector <std::string> Params;
			for (int i = 0; i < Func.Arity; i++)
			{	Params.push_back(JustGet(Buffer.back())); Buffer.pop_back();}
			std::reverse(Params.begin(), Params.end());
			auto& Ret = FuncFunc.Eval(Params);
			switch (FuncFunc.RetType)
			{
				case OBJTYPE::STR:{
					LogicalParser::Literal RetLit;
					RetLit.LiteralValue = Ret;
					Buffer.emplace_back(RetLit, LogicalParser::UnitType::LIETARL);
					break;
				}
				case OBJTYPE::NUM_DOTTED: {
					LogicalParser::NumericalValue RetNum;
					RetNum.Value = Ret;
					RetNum.isDotted = true;
					Buffer.emplace_back(RetNum, LogicalParser::UnitType::NUM);
					break;
				}
				case OBJTYPE::NUM: {
					LogicalParser::NumericalValue RetNum;
					RetNum.Value = Ret;
					RetNum.isDotted = false;
					Buffer.emplace_back(RetNum, LogicalParser::UnitType::NUM);
					break;
				}
				case OBJTYPE::UNDECLARED: {
					break;
				}
			}
		}
		
	};

	for (auto Obj : List)
	{
		switch (Obj.second)
		{
		case LogicalParser::UnitType::LIETARL:
		case LogicalParser::UnitType::NUM:
		case LogicalParser::UnitType::VAR: {
			Buffer.push_back(Obj);
			break;
		}
		case LogicalParser::UnitType::OP:
		{
			ProcessOperator(std::get<LogicalParser::Operator>(Obj.first).Op);
			break;
		}

		case LogicalParser::UnitType::FUNC:
		{
			ProcessFunction((std::get<LogicalParser::Function>(Obj.first)));
			break;
		}
		default: {
			throw std::exception("The world is not ready!");
		}
		}

	}
	if (Buffer.size() == 1)
	{
		m_printer(JustGet(Buffer.front()));
	}
}

void Evaluator::CreateDefaultEnviroment()
{
	using FuncType = std::function<std::string(std::vector<std::string>&)>;

	//Creates new string with the name of the provided literal and default value in second provided parameter
	FuncType NewStr = [this](std::vector<std::string>& In) -> std::string {
		auto& Result = Lexical.Parse(In[0]);
		if ((Result.size() != 1) ||
			(Result.front().size() != 1) ||
			(Result.front().front().Type != LexicalParser::Lexema::NAME)) { throw std::exception("First parameter must be variable name"); }

		auto& LogResult = Logical.Parse(Result);

		if ((LogResult.size() != 1) ||
			(LogResult.front().size() != 1) ||
			(LogResult.front().front().second != LogicalParser::UnitType::VAR)) {
			throw std::exception("First parameter must be variable name");
		}
		return CreateVariable(std::get<LogicalParser::VariableName>(LogResult.front().front().first).Hierarchy, OBJTYPE::STR, In[1]) ?  "1" : "0";
	};
	CreateFunction({ "sys","NewStr" }, OBJTYPE::NUM, NewStr, 2, false);

	//Creates new Numeric with the name of the provided literal and default value in second provided parameter
	FuncType NewNum = [this](std::vector<std::string>& In) -> std::string {
		auto& Result = Lexical.Parse(In[0]);
		if ((Result.size() != 1) ||
			(Result.front().size() != 1) ||
			(Result.front().front().Type != LexicalParser::Lexema::NAME)) {
			throw std::exception("First parameter must be variable name");
		}

		auto& LogResult = Logical.Parse(Result);

		if ((LogResult.size() != 1) ||
			(LogResult.front().size() != 1) ||
			(LogResult.front().front().second != LogicalParser::UnitType::VAR)) {
			throw std::exception("First parameter must be variable name");
		}
		return CreateVariable(std::get<LogicalParser::VariableName>(LogResult.front().front().first).Hierarchy, OBJTYPE::NUM, In[1]) ? "1" : "0";
	};
	CreateFunction({ "sys","NewNum" }, OBJTYPE::NUM, NewNum, 2, false);

	//Creates new DottedNumeric with the name of the provided literal and default value in second provided parameter
	FuncType NewNumDot = [this](std::vector<std::string>& In) -> std::string {
		auto& Result = Lexical.Parse(In[0]);
		if ((Result.size() != 1) ||
			(Result.front().size() != 1) ||
			(Result.front().front().Type != LexicalParser::Lexema::NAME)) {
			throw std::exception("First parameter must be variable name");
		}

		auto& LogResult = Logical.Parse(Result);

		if ((LogResult.size() != 1) ||
			(LogResult.front().size() != 1) ||
			(LogResult.front().front().second != LogicalParser::UnitType::VAR)) {
			throw std::exception("First parameter must be variable name");
		}
		return CreateVariable(std::get<LogicalParser::VariableName>(LogResult.front().front().first).Hierarchy, OBJTYPE::NUM_DOTTED, In[1]) ? "1" : "0";
	};
	CreateFunction({ "sys","NewNumDot" }, OBJTYPE::NUM, NewNumDot, 2, false);

	//Allow any parameter to be consumedm without printing anything
	FuncType Silence = [this](std::vector<std::string>& In) -> std::string {
		return "";
	};
	CreateFunction({ "sys","Silence" }, OBJTYPE::UNDECLARED, Silence, 999, false);
}

bool Evaluator::CreateVariable(std::vector<std::string> VarName, OBJTYPE VarType, std::string defValue, bool CanBeAugmented,std::function<void(std::string)> setter, std::function<std::string()> getter)
{
	switch (VarType)
	{
	case OBJTYPE::UNDECLARED:
		case OBJTYPE::VAR:
		{throw std::exception("Variable can't have that type!"); }
		default: {break; }
	}
	auto& InResult = Sub.emplace(VarName[0], Holder());

	for (int i = 1; i < VarName.size(); i++)
	{
		if (!InResult.first->second.canBeComplex) { throw std::exception("Can't add this variable. It wants to be in object, that dont want anyone to be inside it!"); }
		InResult = InResult.first->second.Sub.emplace(VarName[i], Holder());
	}
	if (!InResult.second) { return false; }

	Variable Var;
	Var.Type = VarType;
	Var.m_set = setter;
	Var.m_get = getter;
	Var.set(defValue);
	InResult.first->second = std::move(Holder(Var));
	InResult.first->second.canBeComplex = CanBeAugmented;
	return true;
}

bool Evaluator::CreateFunction(std::vector<std::string> VarName, OBJTYPE ReturnType, std::function<std::string(std::vector<std::string>&)> Func, int Arity, bool CanBeAugmented)
{
	auto& InResult = Sub.emplace(VarName[0], Holder());

	for (int i = 1; i < VarName.size(); i++)
	{
		if (!InResult.first->second.canBeComplex) { throw std::exception("Can't add this variable. It wants to be in object, that dont want anyone to be inside it!"); }
		InResult = InResult.first->second.Sub.emplace(VarName[i], Holder());
	}
	if (!InResult.second) { return false; }

	Function NewFunc;
	NewFunc.Eval = Func;
	NewFunc.Arity = Arity;
	NewFunc.RetType = ReturnType;
	InResult.first->second = std::move(Holder(NewFunc));
	InResult.first->second.canBeComplex = CanBeAugmented;	
	return true;
}

Evaluator::Evaluator()
{
	CreateDefaultEnviroment();
}

void Evaluator::Parse(std::string Input)
{
	
	try {
		auto& ParseRes = Logical.Parse(Lexical.Parse(Input));
		//Logical.Print(ParseRes);
		for (auto& Command : ParseRes)
		{
			ParseCommand(Command);
		}
	}
	catch (std::exception &e)
	{
		m_printer(std::string(e.what()));
	}
}
