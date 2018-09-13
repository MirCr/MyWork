#pragma once
#include<string>


class Host
{
private:
	std::string name = NULL;
	//bool online = NULL;
	// file FOTO_PROFILO;
public:
	Host();
	~Host();

	void setName(std::string _name);
	std::string getName();
};

