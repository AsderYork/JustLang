#pragma once
#include <any>
#include <vector>
#include <string>
#include <functional>


class ThatIs
{
public:
	struct HoldInterface;
	std::vector<std::pair<std::any, HoldInterface>> Vec;

	template<typename T>
	struct Hold
	{
		std::function<T(std::string)> ToVal;
		std::function<void(T)> Setter;
		std::function<T()> Getter;
		std::function<std::string(T)> FromVal;
		Hold(std::function<T(std::string)> Tv, std::function<void(T)> St, std::function<T()> Gt, std::function<std::string(T)> Fv) :
			ToVal(Tv),
			Setter(St),
			Getter(Gt),
			FromVal(Fv)
			{
			printf("Hold/Construct:%p\n", this);
		}
		void SetVal(std::string val) { Setter(ToVal(val)); }

		~Hold() { printf("Hold/Destruct:%p\n",this); }
	};

	struct HoldInterface
	{

		HoldInterface() { printf("HoldInterface/Construct:%p\n", this); }
		~HoldInterface() { printf("HoldInterface/Destruct:%p\n", this); }
		std::function<void(std::string)> Set;

		template<typename T>
		void SetSet(std::any& h)
		{
			auto& val = (std::any_cast<Hold<T>&>((h)));
			Set = std::function<void(std::string)>(std::bind(&ThatIs::Hold<T>::SetVal, &val, std::placeholders::_1));
		}
	};

	template<typename T>
	void Push(Hold<T>& H) {
		auto Any = std::any(H);
		//HoldInterface Interface;
		//Interface.SetSet<T>(Any);
		Vec.emplace_back(std::move(Any), std::move(HoldInterface()));
		Vec.front().second.SetSet<T>(Vec.front().first);
	}


	void Set(int i, std::string val) {
		try {
			Vec[i].second.Set(val);
		}
		catch (std::exception& e)
		{
			printf("Exception:%s\n", e.what());
		}
	}

};