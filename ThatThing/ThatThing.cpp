// Example program
#include <iostream>
#include "ConsoleParser.h"

class TestOb
{
private:
	int m_val = 12;
public:
	const int get() const{ return m_val; }
	void set(int v) { m_val = v; }

	void show() { printf("TestOb/int:%i\n", m_val); }

	TestOb() { printf("TestObj/Construct:%i\n", m_val); }
	~TestOb() { printf("TestObj/Destruct:%i\n", m_val); }
};

/**!
TYPES
A NAME can contain different things. So here's a list, of what this things could be exactly
NAME-NUMERIC - Contains a numeric value.
NAME-STR - Contains a string value.
NAME-FUNC - Contains a function. Functions can be called.

default type is NAME-STR

*/

/**!
Language notation
[NAME] - A lexema of type NAME, that SHOULD be in that place
(NUMBER)- A lexema of type NUMBER, that MAY be in that place
(NAME/NUMBER) - A lexema of type NAME or NUMBER that should be in that place
<[NUMBER][MARK]...> - Repeat pattern.
In other words, everything, that is inside [] SHOULD be in that place and everything that's in () MAY be there

What can be a command?
[NAME] -This means that A variable with that name must be printed out. If there is no variable with that name, then it's an error

[num/str/func] [NAME] - Create new variable with that name. If variable with that name allready exists, then it's an error

<[NUMBER/NAME-NUMERIC][MATH-MARK]...>[NUMBER/NAME-NUMERIC] - Evaluation pattern. Everything in that pattern should be evaluated as math function.
The Result should be treated also as a Number. Errors may occur during evaluation. This can be referenced later as EVAL-NUMERIC

[NAME-NUMERIC][=][NUMBER/NAME-NUMERIC] - Assignement; If right argument is NAME-NUMERIC. All NAME-NUMERICs here must be valid.

[NAME-STR] = [NAME-STR/LITERAL] - Assignment; Once again, everything should be valid.

[str][(][EVAL-NUMERIC][)] - Evaluates EVAL-NUMERIC and then treats it as LITERAL
[num][(][NAME-STR/LITERAL/NAME-NUMERIC][)] - Casts to numeric. If cast is not possible, it's an error




*/


int main()
{
	TestOb Obh;
	ConsoleParser Parser;

	LexicalParser TextParser;
	LogicalParser LogicParser;

	Parser.RegisterParameter<TestOb, int>(&Obh,"TestObj.m_val", &TestOb::get, &TestOb::set);

	TextParser.Parse("Val=f(fu(8,val+3)+4)");
	LogicParser.Parse(TextParser);
	printf("\n A LOGICAL PARSER!!!\n\n");
	LogicParser.Print();
	return 0;

}
