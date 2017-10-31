#pragma once
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <variant>
#include <optional>
#include <list>

class CP_Subject;

class LexicalParser;
class LogicalParser;

/**!
	This class provides text-based access to internals of the engine
	*/
	class ConsoleParser
	{
	private:
		struct ParameterInterface
		{
			std::function<std::string(std::string)> setVal;//Checks if value can be set, and if it is, then sets it
			std::function<std::string()> getVal;//Returns a value
		};

		std::function<void(std::string)> m_printer;
		std::unordered_map<std::string,ParameterInterface> m_arameters;

	public:
		ConsoleParser(std::function<void(std::string)> Printer = [](std::string Str) {printf("%s\n", Str.c_str());})
			:m_printer(Printer)
		{}

		

		template<typename T>
		static T DefualtTranslator(std::string str)
		{
			static_assert(false, "There is no default translator, switable for this type");
		}
		template<>
		static int DefualtTranslator(std::string str)
		{
			return std::stoi(str);
		}
		template<>
		static float DefualtTranslator(std::string str)
		{
			return std::stof(str);
		}
		template<>
		static bool DefualtTranslator(std::string str)
		{
			if (str.compare("true") == 0) { return true; }
			return false;
		}

		
		template<class ParamClass, typename T>
		void RegisterParameterWithChecker(ParamClass* ObjPointer, std::string ParamName,
			const T(ParamClass::*getter)() const,
			void (ParamClass::*setter)(T),
			std::function<std::string(T)> Checker = nullptr,
			std::function<T(std::string)> ValFromString = DefualtTranslator<T>,
			std::function<std::string(T)> ValToString = [](T val) {return std::to_string(val);})
		{
			RegisterParameter<ParamClass, T>(ObjPointer, ParamName, getter, setter, ValFromString, ValToString, Checker);
		}

		template<class ParamClass, typename T>
		void RegisterParameter(ParamClass* ObjPointer, std::string ParamName,
			const T(ParamClass::*getter)() const,
			void (ParamClass::*setter)(T),
			std::function<T(std::string)> ValFromString = DefualtTranslator<T>,
			std::function<std::string(T)> ValToString = [](T val) {return std::to_string(val);},
			std::function<std::string(T)> Checker = nullptr)
		{
			ParameterInterface Interface;
			Interface.getVal = [=]() {
				return ValToString(std::bind(getter, ObjPointer)());
			};

			Interface.setVal = [=](std::string Str) {
				auto Val = ValFromString(Str);
				if (Checker)
				{
					std::string CheckerResult = Checker(Val);
					if (CheckerResult.size() != 0) { return CheckerResult; }
				}
				std::bind(setter, ObjPointer, Val)();
				return std::string("Value is set!");
			};

			auto InsertResult = m_arameters.insert(std::make_pair(ParamName, Interface));
			if (!InsertResult.second)
			{
				m_printer("Can't bind parameter'"+ ParamName +"': parmeter with the same name is allready exist");
			}
		}

		void RemoveParam(std::string Param)
		{
			auto Interface = m_arameters.find(Param);
			if (Interface == m_arameters.end()) { m_printer("Can't erase parameter. '" + Param + "' doesn't exist!"); return; }
			m_arameters.erase(Interface);
		}

		void PrintVal(std::string Param)
		{
			auto Interface = m_arameters.find(Param);
			if (Interface == m_arameters.end()) { m_printer("Parameter '" + Param + "' doesn't exist!"); return; }
			m_printer(Param + ":" + Interface->second.getVal());
		}

		void SetVal(std::string Param, std::string Val)
		{
			auto Interface = m_arameters.find(Param);
			if (Interface == m_arameters.end()) { m_printer("Parameter '" + Param + "' doesn't exist!"); return; }
			std::string Responce;
			try{
				Responce = Interface->second.setVal(Val);
			}
			catch (std::exception &e)
			{
				Responce = "Failed to set '" + Param + "'; " + e.what();
			}
			if (Responce.size() == 0) { Responce = "Parameter '" + Param + "' successfully set"; }
			m_printer(Responce);
		}
		
	private:

	};

	class ConsoleTextParser {
	private:
		static std::vector<std::string> Keywords;
		
		
		//std::unordered_map<std::string, Object> m_variables;

		/**
		A Lexeme rule. Every simbol have some lexical releation.
		Multiple symbols with same lexical relation forms one lexema.
		If after a line of symbols with one lexical relation, comes a symbol with other lexical relation, it determines a new lexema
		or considered a continuation of a current lexema
		Here are a list of such metamorphoses.
		(CurrentLexicalRelation)+(NewLexicalRelation)=(Continuation/new lexema)
		NAME+NUMBER=Continuation
		NMBER+MARK(.,-)=Continuation
		Everything else is a new lexema
		*/

				

		//std::vector<std::string> parseVariableHierarchy(std::vector<Lexema> Lexems);
		//void addNewVariable(std::vector<std::string> Hierarchy, std::string type, std::string Val);


	public:
		//void AddVariable(std::string Name, std::string type = "str", std::string Value = "");

		//void PrintAllVariables(std::unordered_map<std::string, Object>* map = nullptr, std::string Prefix = "");
	
	};

	class LexicalParser
	{
	private:
		friend class LogicalParser;
		struct Lexema
		{
			/**!
			Not all marks can have a meaning. There's a vector of ones that do
			*/
			static std::vector<std::string> AllowedMarks;
			enum LEXEME_TYPE {
				NONE,
				NAME,//"A_name","Name34", "Ultra name"; Everything, that is a name of something. Or just a string of text
				NUMBER,//"5"
				NUMBER_DOTTED,
				MARK,
				LITERAL
			} Type;

			std::string str;
			Lexema(LEXEME_TYPE Type, std::string& _str) : Type(Type), str(_str) {}
			Lexema() : Type(NONE), str() {}
		};
		//A state machine to parse a command;

		Lexema m_tmpLexema;
		std::vector<std::vector<Lexema>> m_parsedLexemas;

		void PushTmpLexema();
		void ParseOneCharacter(char& c);
		std::string ParseOneCommand(std::string& line);
	public:
		
		std::vector<std::vector<Lexema>>& GetLexicalTable() { return m_parsedLexemas; }
		void Parse(std::string line);
	};

	/**LOGICAL PARSER THINGS*/
	class LogicalParser
	{
	private:
		//Pre-declaration;
		struct VariableName;
		struct NumericalValue;
		struct Literal;
		struct Function;
		struct Operator;
		struct EVAL_Num;
		struct EVAL_Str;
		enum class UnitType { VAR, NUM, FUNC, OP, LIETARL};//Variable, Numerical, Function, Operator, ...
		using LogicalUnit = std::pair<std::variant<VariableName, NumericalValue, Literal, Function, Operator>, UnitType>;

		struct Object {
			enum class NAME_TYPE {
				STR,
				NUM,
				FUNC,
			} Subtype = NAME_TYPE::STR;
			std::unordered_map<std::string, Object> Subobjects;
			std::string Value;
		};



		

		struct VariableName {
			/**
			<[NAME][.]...>[NAME] - A variable name
			*/
			std::vector<std::string> Hierarchy;
		};
		struct NumericalValue {
			std::string Value;
			bool isDotted = false;
		};
		struct Literal {
			std::string LiteralValue;
		};
		struct Operator{
			std::string Op;
			bool isActionOperator() const;
		};
		//Complex units; They can be deduced only after all previous units. They can remove units from m_units and use them inside!
		struct EVAL_Num {

		};
		struct Function {
			//[[VAR][(](VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC)[)] / [VAR][(][VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC]<[,][VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC]>[)]
			int Arity=0;
			VariableName Name;
		};


		std::vector<std::vector<LogicalUnit>> m_units;

		/*Checks if Buffer contains a LogicalUnit of a given type. If it does,
		It will create an instance of it and return it. Otherwise nothing will be returned
		The second value will be in (From,Lexemas.size()]. It's a place of a last lexema, used to construct A Varname.
		If no VarName returned, then this value will be -1;		*/
		std::pair<std::optional<VariableName>,int> IsThereVariableName(std::vector<LexicalParser::Lexema>& Lexemas, int From);
		//Check if Lexema[From] Is numerical. If it is builds and returns it
		std::optional<NumericalValue> IsThereNumerical(std::vector<LexicalParser::Lexema>& Lexemas, int From);
		//Check if Lexema[From] Is literal. If it is builds and returns it
		std::optional<Literal> IsThereLiteral(std::vector<LexicalParser::Lexema>& Lexemas, int From);
		//Check if Lexema[From] Is operator. If it is builds and returns it
		std::optional<Operator> IsThereOperator(std::vector<LexicalParser::Lexema>& Lexemas, int From);

		std::list<LogicalUnit> ParseCommand(std::vector<LexicalParser::Lexema>& Lexemas);

		//Collapse [VAR][(] in to [FUNCTION]
		void CollapseFunctions(std::list<LogicalUnit>& Units);
		std::vector<LogicalUnit> ToPostfix(std::list<LogicalUnit>& Units);

		
	public:

		void Parse(LexicalParser& LexicalParser);
		void Print();

	};