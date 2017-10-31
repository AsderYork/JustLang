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
std::vector<std::string> ConsoleTextParser::Keywords = {
	"str", "num", "func"//Types
};


/*
std::vector<std::string> ConsoleTextParser::parseVariableHierarchy(std::vector<Lexema> Lexems)
{
	std::vector<std::string> ObjectHierarchy;
	int Level = 0;
	for (auto& Lex : Lexems)
	{
		if (Lex.str == ".") { Level++; }
		else if (Lex.Type == Lexema::NAME)
		{
			if (ObjectHierarchy.size() != Level) { throw std::exception("Failed to parse variable name. It's hierarchy is broken!"); }
			else
			{
				ObjectHierarchy.push_back(Lex.str);
			}
		}
		else { throw std::exception("Failed to parse variable name. Variable name contains something, that is not a part of it!"); }
	}
	if (ObjectHierarchy.size() == 0)
	{
		throw std::exception("Failed to parse variable name. There is no actual name!");
	}

	return ObjectHierarchy;
}
*/
/*
void ConsoleTextParser::addNewVariable(std::vector<std::string> Hierarchy, std::string type, std::string Val)
{
	bool IsAnythingAdded = false;//It is possible, that specified variable is allready exists. This is an error!

	auto InsertResult = m_variables.insert(std::make_pair(Hierarchy[0], Object()));
	if (InsertResult.second) { IsAnythingAdded = true; }

	if (Hierarchy.size() > 1)
	{
		for (int i = 1; i < Hierarchy.size(); i++)
		{
			InsertResult = InsertResult.first->second.Subobjects.insert(std::make_pair(Hierarchy[i], Object()));
			if (InsertResult.second) { IsAnythingAdded = true; }
		}
	}
	if (!IsAnythingAdded) { throw std::exception("This variable is allready exist!"); }

	InsertResult.first->second.Value = Val;
	if (type == "num") { InsertResult.first->second.Subtype = Object::NAME_TYPE::NUM; }
	else if (type == "str") { InsertResult.first->second.Subtype = Object::NAME_TYPE::STR; }
	else if (type == "func") { InsertResult.first->second.Subtype = Object::NAME_TYPE::FUNC; }
	else if (type.size() == 0) { throw("Provided type is not identified"); }
	//Otherwise type is default. And it's str

}
*/

/*
void ConsoleTextParser::AddVariable(std::string Name, std::string type, std::string Value)
{
	
	try {
		if (Name.find(";") != std::string::npos)
		{
			throw std::exception("Variable name contain ';'");
		}

		CommandParser Parser;
		Parser.Parse(Name);

		auto PrasedRes = parseVariableHierarchy(Parser.m_parsedLexemas);

		addNewVariable(PrasedRes, type, Value);
	}
	catch (std::exception&e)
	{
		printf("%s\n", e.what());
	}
	
}
*/
/*
void ConsoleTextParser::PrintAllVariables(std::unordered_map<std::string, Object>* map, std::string Prefix)
{
	if (map == nullptr) { map = &m_variables; }

	for (auto& Obj : *map)
	{
		printf("%s%s\n", Prefix.c_str(), Obj.first.c_str());
		Prefix = Obj.first + ".";
		PrintAllVariables(&(Obj.second.Subobjects), Prefix);
	}

}*/

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

std::vector<LogicalParser::LogicalUnit> LogicalParser::ToPostfix(std::list<LogicalUnit>& Units)
{
	std::vector<LogicalUnit> Stack;
	std::vector<LogicalUnit> Result;

	auto IncrementArityInTopFunction = [&]() {
		for (auto& rit = Stack.rbegin(); rit != Stack.rend(); rit++)
		{
			if (rit->second == UnitType::FUNC) {
				std::get<Function>(rit->first).Arity++; return;
			}
		}
	};
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
			{ Result.push_back(Stack.back()); Stack.pop_back(); return; }
			Result.push_back(Stack.back()); Stack.pop_back();
		}
		//If we're still here
		throw std::exception("Too much ')'");
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
				Stack.push_back(unit);
				break;
			}
			case UnitType::OP: {
				auto& OpRep = std::get<Operator>(unit.first);
				if (OpRep.Op == ",") { IncrementArityInTopFunction(); }
				else if (OpRep.Op == ")") { PushOutOfStuckUntillFunctionOrParentheses(); }
				else if (OpRep.isActionOperator()) { Stack.push_back(unit); }
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
			default:
				throw("Logical unit misses it's type!");
			}
		}
		printf("\n");
	}
}

bool LogicalParser::Operator::isActionOperator() const
{
	static std::vector<std::string> ActionOps{
		"+", "-", "*", "/",
		"=", "+=", "*=", "-=", "/=", "|=", "&=", "^=",
		"!", "==", "<", ">", "<=", ">="
	};

	return std::find(ActionOps.begin(), ActionOps.end(), Op) != ActionOps.end();
}