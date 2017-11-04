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
Language things!
There are 3 fundamental types.
[NUM] - Numerics. It's integers.
[NUM_D] - Dotted numerics. It's floating point numbers.
[LITERAL] - Literals. Its strings.

Different actions can be performed on them.
Perfoming of operation cause Collapse. It's when arguments of operation and operation itself is replaced with it's result.
[NUM][[+]/[-]/[*]/[/]][NUM]. Performs calculation. Returns one [NUM] that is a result of this operation.
[LITERAL][+][LITERAL]. Concatenates two strings. Result is aslo [LITERAL].
[NUM][[<]/[>]/[==]/[<=]/[>=]][NUM]. Returns [NUM]. 1 if expression is true. 0 otherwise.

There are also Variables [VAR]. Variables can be one of 3 types above. They can take place in expressions above, if they have correnct type



*/

int main()
{
	
	TestOb Obh;
	ConsoleParser Parser;

	Evaluator Evlauatie;

	Parser.RegisterParameter<TestOb, int>(&Obh,"TestObj.m_val", &TestOb::get, &TestOb::set);

	//Evlauatie.Parse("sys.Silence(sys.NewNum(\"Result\", 2+(10*4)));Result;");
	//Evlauatie.Parse("\"Lit\\\"Q\\\"W\ \"");
	std::string Command = "\\\"Hello world!\\\"";
	Evlauatie.Parse("sys.NewStr(\"Seq\",\" "+ Command+ " \")");
	while (true)
	{
		char input[512];
		std::cin.getline(input, sizeof(input));
		Evlauatie.Parse(input);
	}

	return 0;

}
