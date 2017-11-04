#pragma once
#include <functional>
#include <tuple>
#include <array>
#include <type_traits>

template<typename...T>
void Do(T...t)
{};


template<int>
struct My_2 {
};

namespace std {
	template<int N>
	struct is_placeholder<My_2<N>> : public integral_constant<int, N+1> {};
}

template<typename T>
T ParamToStr(std::string val)
{}
template<>
int ParamToStr(std::string val)
{
	printf("This is int!\n");
	return std::stoi(val);
}
template<>
bool ParamToStr(std::string val)
{
	printf("This is bool!\n");
	return val == "1" ? true : false;
}
template<>
float ParamToStr(std::string val)
{
	printf("This is float!\n");
	return std::stof(val);
}
template<>
std::string ParamToStr(std::string val)
{
	printf("This is string!\n");
	return val;
}

template<typename T>
std::string ResultToStr(T val){ return std::to_string(val); }




void TestFunc(int x, int y) { printf("<%i,%i>\n", x, y); }

void Mn()
{
	//std::bind(TestFunc, My_2<1>(), 2)(2, 4);
}


template<class ParamClass, typename T, std::size_t...I,typename... Args>
decltype(auto) RegisterObjectFunctionImpl(std::index_sequence<I...>,ParamClass* Obj, T(ParamClass::*Func)(Args...))
{
	auto StatFunc = std::bind(Func, Obj, std::bind(ParamToStr<Args>, My_2<I>())...);


	auto Lambda = [StatFunc](std::vector<std::string>& Params) {
		return ResultToStr(std::bind(StatFunc, Params[I]...)());
	};
	return std::function<std::string(std::vector<std::string>&)>(Lambda);
};

template<class ParamClass, std::size_t...I, typename... Args>
decltype(auto) RegisterObjectFunctionImpl(std::index_sequence<I...>, ParamClass* Obj, void(ParamClass::*Func)(Args...))
{
	auto StatFunc = std::bind(Func, Obj, std::bind(ParamToStr<Args>, My_2<I>())...);


	auto Lambda = [StatFunc](std::vector<std::string>& Params) {
		std::bind(StatFunc, Params[I]...)();
		return std::string();
	};
	return std::function<std::string(std::vector<std::string>&)>(Lambda);
};



template<class ParamClass, typename T, typename... Args>
decltype(auto) RegisterObjectFunction(ParamClass* Obj, T(ParamClass::*Func)(Args...))
{
	using Indices = std::make_index_sequence<sizeof...(Args)>;
	return std::make_pair(RegisterObjectFunctionImpl(Indices{}, Obj, Func), sizeof...(Args));

};