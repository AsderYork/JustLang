#pragma once
#include <string>
#include <functional>
#include <vector>
#include <map>
#include <unordered_map>
#include <variant>
#include <optional>
#include <list>

class CP_Subject;

class LexicalParser;
class LogicalParser;
class Evaluator;

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
		friend class Evaluator;
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

		using RetType = std::vector<std::vector<Lexema>>;
		void PushTmpLexema(RetType& TmpRet, Lexema& TmpLexema);
		void ParseOneCharacter(char& c, RetType& TmpRet, Lexema& TmpLexema);
		std::string ParseOneCommand(std::string& line, RetType& TmpRet, Lexema& TmpLexema);
	public:
		std::vector<std::vector<Lexema>> Parse(std::string line);
	};

	/**LOGICAL PARSER THINGS*/
	class LogicalParser
	{
	private:
		friend class Evaluator;
		//Pre-declaration;
		struct VariableName;
		struct NumericalValue;
		struct Literal;
		struct Function;
		struct Operator;		
		enum class UnitType { VAR, NUM, FUNC, OP, LIETARL, NONE};//Variable, Numerical, Function, Operator, ...
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
			int getPriority() const;
			bool isActionOperator() const;
		};
		//Complex units; They can be deduced only after all previous units. They can remove units from m_units and use them inside!
		struct Function {
			//[[VAR][(](VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC)[)] / [VAR][(][VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC]<[,][VAR/NUMERIC/EVAL_NUM/EVAL_STR/LITERAL/FUNC]>[)]
			int Arity=0;
			VariableName Name;
		};


		//std::vector<std::list<LogicalUnit>> m_units;

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
		std::list<LogicalUnit> ToPostfix(std::list<LogicalUnit>& Units);

		
	public:

		std::vector<std::list<LogicalUnit>> Parse(std::vector<std::vector<LexicalParser::Lexema>>& Lexemics);
		void Print(std::vector<std::list<LogicalUnit>>& Input);

	};

	class Evaluator
	{
		/**!Evaluation rules
		[VAR]
		Shows variable value; Variable must presist in a ScopeStack;

		[NUM/LITERAL]
		Shows a value of NUM or Literal;

		[VAR]=[NUM/LITERAL/VAR]
		Assign value of right part to the Variable on the left.
		Type of Variable on the right must match type of something on the right
		All variables in this command must presist in a ScopeStack;

		[{]<COMMAND>[}]
		A block. Creates it's own scope. Can be used as a single COMMAND

		[if][(][NUM][)][COMMAND]
		Process next command only if NUM != 0; Yes, there is no bool type.
		
		[while][(][NUM][)][COMMAND]
		Process next command while NUM == 0; Please, don't do infinite loop! There is no way to check for it. It's Turing-complete!
		
		[FUNC][(]([VAR/LITARAL/NUM]<[,][VAR/LITEARL/NUM]>)[)]
		Calls a function. Function can be provided from somewhe. If there is no function with specified name, check str Variables.
		If there is str Variable wth name of a specified function, evaluate it. But they should not have any input! They also can't have output!

		There was a rule on how to create a variable. Well, no more! Now only using sys.createStr([NAME],[VAL])/sys.createNum([NAME],[VAL]);
		*/	
	private:

		std::function<void(std::string&)> m_printer = [](std::string& s) {printf("%s\n", s.c_str());  };

		 enum class OBJTYPE { STR, NUM, NUM_DOTTED, UNDECLARED, VAR };//VAR - can be used only as a return type for Functions!
		//Something, that can be accessed. It can recive
		struct Variable {
			OBJTYPE Type;
			std::string Value;
			//This methods can't be assigned in Language, but other code can use it to allow Language to manage it's variables as a regular variable
			std::function<void(std::string)>m_set;
			std::function<std::string()>m_get;

			void set(std::string Val){
				if (m_set) { m_set(Val); }	else { Value = Val; }
			}
			std::string get() {
				if (m_get) { return m_get(); }
				else { return Value; }
			}
		
		};
		struct Function {
			//Functions can't realy be set in language. But they can be accessed by other code!
			OBJTYPE RetType;//If function returns UNDECLARED then it returns nothing
			std::function<std::string(std::vector<std::string>&)>Eval;
			int Arity = 0;//Amount of values function recives. If this value is bigger, then was passed, missing value will be empty strings
		};
		struct Holder {
		private:
			std::variant<Variable, Function> Holding;
		public:
			bool canBeComplex = true;//True if new objects can be added to this one. False otherwise
			Holder(std::variant<Variable, Function>& Obj) { Holding = std::move(Obj); }
			Holder(Variable& Obj) : Holding(std::move(Obj)) {}
			Holder(Function& Obj) : Holding(std::move(Obj)) {}
			Holder() {}
			Holder(const Holder&& Obj) : Holding(std::move(Obj.Holding)) {}
			Holder& operator=(const Holder&& Obj) { Holding = std::move(Obj.Holding); return *this; }

			template<typename T>
			T& get() {return std::get<T>(Holding); }

			std::map<std::string,Holder> Sub;
			template<typename T>
			bool isType() const { return std::holds_alternative<T>(Holding); }
		
		};

		std::map<std::string, Holder> Sub;

		std::optional<std::map<std::string, Evaluator::Holder>::iterator> FindName(const std::vector<std::string>& Name);

		void ParseCommand(std::list<LogicalParser::LogicalUnit>& List);

		void CreateDefaultEnviroment();

		//Creates variable.
		bool CreateVariable(std::vector<std::string> VarName, OBJTYPE VarType, std::string defValue = "", bool CanBeAugmented = true, std::function<void(std::string)>setter = nullptr,	std::function<std::string()>getter = nullptr);

		bool CreateFunction(std::vector<std::string> VarName, OBJTYPE ReturnType, std::function<std::string(std::vector<std::string>&)>Func, int Arity, bool CanBeAugmented = true);

		LexicalParser Lexical;
		LogicalParser Logical;

	public:
		Evaluator();
		void Parse(std::string Input);
	};