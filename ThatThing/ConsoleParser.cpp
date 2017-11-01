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
std::vector<std::string> LexicalParser::Lexema::UsedTypes = { "str","num" };
std::vector<std::string> LexicalParser::Lexema::Keywords = { "if","while" };
std::vector<std::string> ConsoleTextParser::Keywords = {
	"str", "num", "func"//Types
};

void LexicalParser::PushTmpLexema()
{
	if (m_tmpLexema.Type != Lexema::NONE)
	{
		if (m_tmpLexema.str.size() != 0)
		{
			m_parsedLexemas.back().push_back(m_tmpLexema);
			m_tmpLexema.str.clear();
		}
		m_tmpLexema.Type = Lexema::NONE;
	}
}

void LexicalParser::ParseOneCharacter(char & c)
{
	if (m_tmpLexema.Type == Lexema::LITERAL)
	{
		if (c == '"') {PushTmpLexema(); return; }
	}
	else if (isdigit(c))
	{
		if (m_tmpLexema.Type == Lexema::NAME){/*Then it's still a name. Do nothing*/}
		else if ((m_tmpLexema.Type == Lexema::NUMBER) || (m_tmpLexema.Type == Lexema::NUMBER_DOTTED)) {/*Then it's still a number*/}
		else if (m_tmpLexema.str == "-") {
			if ((m_parsedLexemas.size() == 0 )||(m_parsedLexemas.back().back().Type == Lexema::MARK))
			{/*Then this minus is from a number. So just do nothing and let the rest of the number be added to it*/}
			else {/*Then this minus is from other lexema. Push it and start a new one*/
				PushTmpLexema();
			}
			m_tmpLexema.Type = Lexema::NUMBER;
		}
		else
		{
			PushTmpLexema();
			m_tmpLexema.Type = Lexema::NUMBER;
		}
	}
	else if (c == '.')
	{	
		if (m_tmpLexema.Type == Lexema::NUMBER) {m_tmpLexema.Type = Lexema::NUMBER_DOTTED;}
		else if (m_tmpLexema.Type == Lexema::NUMBER_DOTTED) { throw std::exception("A Number have more then one dot!");}
		else
		{
			PushTmpLexema();
			m_tmpLexema.Type = Lexema::MARK;
		}
	}
	else if ((c != '_') && (ispunct(c)))
	{
		if ((c == '-') && (m_tmpLexema.str == "-") && (m_parsedLexemas.back().back().Type == Lexema::NUMBER))
		{//Numbers cannot be decremented. So it's definetly just a minus;
			PushTmpLexema();
			m_tmpLexema.Type = Lexema::MARK;
		}
		else if(m_tmpLexema.Type != Lexema::MARK){ PushTmpLexema();	m_tmpLexema.Type = Lexema::MARK;}
		
		if (std::find(Lexema::AllowedMarks.begin(), Lexema::AllowedMarks.end(), (m_tmpLexema.str + c)) == Lexema::AllowedMarks.end())
		{//Then there is no lexema, that could be fromed adding this character
			if (std::find(Lexema::AllowedMarks.begin(), Lexema::AllowedMarks.end(), m_tmpLexema.str) == Lexema::AllowedMarks.end())
				{
					throw std::exception("A mark is provided, but this particular is not allowed:" + c);
				}
			PushTmpLexema();
			m_tmpLexema.Type = Lexema::MARK;
		}
	}
	else//Then it's a NAME!
	{
		if (m_tmpLexema.Type != Lexema::NAME) { PushTmpLexema(); m_tmpLexema.Type = Lexema::NAME; }
	}
	m_tmpLexema.str += c;
	if (m_tmpLexema.Type == Lexema::NAME) {
		if (std::find(Lexema::UsedTypes.begin(), Lexema::UsedTypes.end(), m_tmpLexema.str) != Lexema::UsedTypes.end())
		{
			m_tmpLexema.Type = Lexema::TYPENAME; PushTmpLexema();
		}
		else if (std::find(Lexema::Keywords.begin(), Lexema::Keywords.end(), m_tmpLexema.str) != Lexema::Keywords.end())
		{
			m_tmpLexema.Type = Lexema::KEYWORD; PushTmpLexema();
		}
	}
}

std::string LexicalParser::ParseOneCommand(std::string &line)
{
	m_parsedLexemas.emplace_back();//Add new command
	std::string RemainingCommand;

	for (int i = 0; i < line.size(); i++)
	{
		if (m_tmpLexema.Type == Lexema::LITERAL){ParseOneCharacter(line[i]);}
		else if (isspace(line[i]))	{ PushTmpLexema(); }
		else if (line[i] == '"')	{ PushTmpLexema(); m_tmpLexema.Type = Lexema::LITERAL;}
		else if (line[i] == ';') { PushTmpLexema(); RemainingCommand = line.substr(i + 1); break; }
		else						{ParseOneCharacter(line[i]);}
	}
	PushTmpLexema();
	return RemainingCommand;
}


void LexicalParser::Parse(std::string line)
{
	try
	{
		while (line.size() != 0)
		{
			line = std::move(ParseOneCommand(line));
			if (m_parsedLexemas.back().size() == 0) { m_parsedLexemas.erase(m_parsedLexemas.end()-1); }
		}
	}
	catch (std::exception &e)
	{
		printf("exception:%s\n", e.what());
	}
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
			case Lexema::LITERAL: { printf("<LIT>"); break; }
			case Lexema::TYPENAME: { printf("<TYP>"); break; }
			case Lexema::KEYWORD: { printf("<LEY>"); break; }
			default:
				break;
			}
			printf("%s|", Liter.str.c_str());
		}
		printf("\n");
	}

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

std::optional<LogicalParser::TYPE> LogicalParser::IsThereType(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	TYPE PotentialObject;
	if (Lexemas[From].Type == LexicalParser::Lexema::TYPENAME)
	{
		PotentialObject.str = Lexemas[From].str;
		return PotentialObject;
	}
	return std::nullopt;
}

std::optional<LogicalParser::KEYWORD> LogicalParser::IsThereKeyword(std::vector<LexicalParser::Lexema>& Lexemas, int From)
{
	KEYWORD PotentialObject;
	if (Lexemas[From].Type == LexicalParser::Lexema::KEYWORD)
	{
		PotentialObject.str = Lexemas[From].str;
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

		auto TypeResult = IsThereType(Lexemas, i);
		if (TypeResult) { RetList.emplace_back(LogicalUnit(TypeResult.value(), UnitType::TYPE)); continue; }

		auto KeywordlResult = IsThereKeyword(Lexemas, i);
		if (KeywordlResult) { RetList.emplace_back(LogicalUnit(KeywordlResult.value(), UnitType::KEYWORD)); continue; }
		
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
		else if (it->second == UnitType::TYPE)
		{
			auto VarIt = it;
			it++;
			if (it == Units.end()) { break; }
			if ((it->second == UnitType::OP) && (std::get<Operator>(it->first).Op == "("))
			{
				TYPE_F Func;
				Func.str = std::get<TYPE>(VarIt->first).str;
				auto NewIt = Units.erase(VarIt, ++it);
				it = Units.insert(NewIt, LogicalUnit(Func, UnitType::TYPE_F));
			}
		}
		else if (it->second == UnitType::KEYWORD)
		{
			auto VarIt = it;
			it++;
			if (it == Units.end()) { break; }
			if ((it->second == UnitType::OP) && (std::get<Operator>(it->first).Op == "("))
			{
				KEYWORD Func;
				Func.str = std::get<KEYWORD>(VarIt->first).str;
				auto NewIt = Units.erase(VarIt, ++it);
				it = Units.insert(NewIt, LogicalUnit(Func, UnitType::KEYWORD));
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
			if (rit->second == UnitType::TYPE_F) {
				if (std::get<TYPE_F>(rit->first).Arity == 0) {
					std::get<TYPE_F>(rit->first).Arity++;
				}
				return;
			}
			if (rit->second == UnitType::KEYWORD) {
				if (std::get<KEYWORD>(rit->first).Arity == 0) {
					std::get<KEYWORD>(rit->first).Arity++;
				}
				return;
			}
		}
	};
	auto PushOutOfStuckUntillFunctionOrParentheses = [&]() {
		
		while (Stack.size() != 0)
		{
			if (Stack.back().second == UnitType::FUNC) { Result.push_back(Stack.back()); Stack.pop_back(); return; }
			else if (Stack.back().second == UnitType::TYPE_F) { Result.push_back(Stack.back()); Stack.pop_back(); return; }
			else if (Stack.back().second == UnitType::KEYWORD) { Result.push_back(Stack.back()); Stack.pop_back(); return; }
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
			else if (Stack.back().second == UnitType::TYPE_F) {
				std::get<TYPE_F>(Stack.back().first).Arity++;  return;
			}
			else if (Stack.back().second == UnitType::KEYWORD) {
				std::get<KEYWORD>(Stack.back().first).Arity++;  return;
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
			case UnitType::TYPE:
			case UnitType::LIETARL: {
				Result.push_back(unit);
				IAITF_ButOnlyIfItsZero();
				break;
			}
			case UnitType::TYPE_F:
			case UnitType::KEYWORD:
			case UnitType::FUNC: {
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

void LogicalParser::Parse(LexicalParser & LexicalParser)
{
	auto& LexTable = LexicalParser.GetLexicalTable();
	for (auto LexList : LexTable)
	{
		auto& List = ParseCommand(LexList);
		CollapseFunctions(List);
		m_units.push_back(ToPostfix(List));
	}
}

void LogicalParser::Print()
{
	for (auto& Command : m_units)
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
			case UnitType::TYPE: {
				printf("<TYPE>");
				printf("%s|", std::get<TYPE>(Unit.first).str.c_str());
				break;
			}
			case UnitType::TYPE_F: {
				printf("<TY_F>");
				printf("%s|", std::get<TYPE_F>(Unit.first).str.c_str());
				break;
			}
			case UnitType::KEYWORD: {
				printf("<KEYW>");
				printf("%s|", std::get<KEYWORD>(Unit.first).str.c_str());
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


std::optional<Evaluator::Holder> Evaluator::FindName(const std::vector<std::string>& Name) const
{
	auto FindResult = Sub.find(Name[0]);
	if (FindResult == Sub.end()) { return std::nullopt; }
	for (int i = 1; i < Name.size(); i++)
	{
		auto End = FindResult->second.Sub.end();
		FindResult = FindResult->second.Sub.find(Name[i]);
		if (FindResult == End) { return std::nullopt; }
	}
	return std::optional<Evaluator::Holder>(FindResult->second);
}

void Evaluator::ParseCommand(std::list<LogicalParser::LogicalUnit>& List)
{
	std::vector<std::pair<LogicalParser::LogicalUnit, Holder*>> Buffer;

	//Binds a variable to Object 
	auto Bind = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {
		if (Obj.second == nullptr)
		{
			std::optional<Holder> Result;
			if (Obj.first.second == LogicalParser::UnitType::VAR)
			{Result = FindName(std::get<LogicalParser::VariableName>(Obj.first.first).Hierarchy);
			if (Result && Result->isType<Function>()) { throw std::exception("Function cannot be used as a variable!"); }
			Obj.second = &(*Result);
			}
			else if (Obj.first.second == LogicalParser::UnitType::FUNC)
			{
				Result = FindName(std::get<LogicalParser::Function>(Obj.first.first).Name.Hierarchy);
				Obj.second = &(*Result);
			}
			if (!Result) { std::exception("Requested object cannot be found!"); }
			return Result;
		}
		return std::optional<Holder>( *(Obj.second));
	};
	//Returns true for Numerics and And Numeric/DotNumeric variables. false otherwise
	auto CanBeNUM = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {
		if (Obj.first.second == LogicalParser::UnitType::NUM) { return true; }
		if (Obj.first.second == LogicalParser::UnitType::VAR) {
			Bind(Obj);
			if(Obj.second->isType<Variable>())
				if(Obj.second->get<Variable>().Type == OBJTYPE::NUM_DOTTED || Obj.second->get<Variable>().Type == OBJTYPE::NUM)
				{return true;}
		}
		return false;
	};
	//Returns a string Rep and isDotted;
	auto GetAsNUM = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {

		if (Obj.first.second == LogicalParser::UnitType::NUM) {
			auto& Sh = std::get<LogicalParser::NumericalValue>(Obj.first.first);
			return std::make_pair(Sh.Value, Sh.isDotted);
		}
		if (Obj.first.second == LogicalParser::UnitType::VAR) {
			Bind(Obj);
			if (Obj.second->isType<Variable>())
				if(Obj.second->get<Variable>().Type == OBJTYPE::NUM_DOTTED){ return std::make_pair(Obj.second->get<Variable>().get(), true); }
				else if (Obj.second->get<Variable>().Type == OBJTYPE::NUM){ return std::make_pair(Obj.second->get<Variable>().get(), true); }
				
		}
		throw std::exception("A value is requested as numeric. But it's not numeric!");
	};

	auto CanBeLiteral = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {
		if (Obj.first.second == LogicalParser::UnitType::LIETARL) { return true; }
		if (Obj.first.second == LogicalParser::UnitType::VAR) {
			Bind(Obj);
			if (Obj.second->isType<Variable>())
				if (Obj.second->get<Variable>().Type == OBJTYPE::STR)
				{return true;}
		}
		return false;
	};
	auto GetAsLiteral = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {

		if (Obj.first.second == LogicalParser::UnitType::LIETARL) {
			return std::get<LogicalParser::Literal>(Obj.first.first).LiteralValue;
		}
		if (Obj.first.second == LogicalParser::UnitType::VAR) {
			Bind(Obj);
			if (Obj.second->isType<Variable>())
			{
				if (Obj.second->get<Variable>().Type == OBJTYPE::STR) { return Obj.second->get<Variable>().get(); }
			}
		}
		throw std::exception("A value is requested as literal. But it's not literal!");
	};
	//Check if it's a variable. If it is, returns it's type
	auto CanBeVariable = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj) {
		if (Obj.first.second == LogicalParser::UnitType::VAR) {
			Bind(Obj);
			auto MB = Bind(Obj);
			if (Obj.second->isType<Variable>())
			{
				return std::optional<OBJTYPE>(Obj.second->get<Variable>().Type);
			}
		}
		return std::optional<OBJTYPE>(std::nullopt);
	};
	auto SetVariable = [&](std::pair<LogicalParser::LogicalUnit, Holder*>& Obj, std::string& val) {
			Obj.second->get<Variable>().set(val);
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
					Numval.Value = std::to_string(std::stof(Left.first)+ std::stof(Right.first));
				}
				else{Numval.Value = std::to_string(std::stoi(Left.first) + std::stoi(Right.first));}
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
			else if (CanBeLiteral(Buffer[Buffer.size() - 1]) && CanBeLiteral(Buffer[Buffer.size() - 2])) {
				LogicalParser::Literal LitVal;
				auto& Left = GetAsLiteral(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsLiteral(Buffer[Buffer.size() - 1]);				
				LitVal.LiteralValue = Left + Right;
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(LitVal, LogicalParser::UnitType::LIETARL), nullptr));
			}
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
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
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
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
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
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
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
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}

		else if (op == "<") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) < std::stof(Right.first) ? 0 : 1);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) < std::stoi(Right.first) ? 0 : 1); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}
		else if (op == ">") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) > std::stof(Right.first) ? 0 : 1);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) > std::stoi(Right.first) ? 0 : 1); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}
		else if (op == "==") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) == std::stof(Right.first) ? 0 : 1);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) == std::stoi(Right.first) ? 0 : 1); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}
		else if (op == "<=") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) <= std::stof(Right.first) ? 0 : 1);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) <= std::stoi(Right.first) ? 0 : 1); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}
		else if (op == ">=") {
			if (CanBeNUM(Buffer[Buffer.size() - 1]) && CanBeNUM(Buffer[Buffer.size() - 2])) {
				LogicalParser::NumericalValue Numval;
				auto& Left = GetAsNUM(Buffer[Buffer.size() - 2]);
				auto& Right = GetAsNUM(Buffer[Buffer.size() - 1]);
				if (Left.second || Right.second)
				{
					Numval.Value = std::to_string(std::stof(Left.first) >= std::stof(Right.first) ? 0 : 1);
				}
				else { Numval.Value = std::to_string(std::stoi(Left.first) >= std::stoi(Right.first) ? 0 : 1); }
				Buffer.pop_back(); Buffer.pop_back();
				Buffer.push_back(std::make_pair(LogicalParser::LogicalUnit(Numval, LogicalParser::UnitType::NUM), nullptr));
			}
		}

		else if (op == "=") {
			if (CanBeVariable(Buffer[Buffer.size() - 2])) {
				if (CanBeLiteral(Buffer[Buffer.size() - 1]) && Buffer[Buffer.size() - 2].second->get<Variable>().Type == OBJTYPE::STR)
				{
					SetVariable(Buffer[Buffer.size() - 2], GetAsLiteral(Buffer[Buffer.size() - 1]));
				}
				else if (CanBeNUM(Buffer[Buffer.size() - 1]))
				{
					auto& Val = GetAsNUM(Buffer[Buffer.size() - 1]);
					if(Buffer[Buffer.size() - 2].second->get<Variable>().Type == OBJTYPE::NUM_DOTTED)
					SetVariable(Buffer[Buffer.size() - 2], std::to_string( std::stof(Val.first)) );
					else if (!Val.second)
					{SetVariable(Buffer[Buffer.size() - 2], std::to_string(std::stoi(Val.first)));}
					else {
						throw std::exception("Dotted value cannot be assigned to a dotted one!");
					}
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
				Buffer.push_back(std::make_pair(Obj, nullptr));
				break;
			}
			case LogicalParser::UnitType::OP:
			{	
				ProcessOperator(std::get<LogicalParser::Operator>(Obj.first).Op); 
				break; 
			}
			default:{
				throw std::exception("The world is not ready!");
			}
		}
		
	}
}

void Evaluator::Parse(LogicalParser & Input)
{
	Variable Val;
	Val.name = "Val";
	Val.Type = OBJTYPE::NUM_DOTTED;
	Sub.insert(std::make_pair("Val", Holder(Val)));

	bool Is = Sub.find("Val")->second.isType<Variable>();

	for (auto& Command : Input.m_units)
	{
		ParseCommand(Command);
	}
}
