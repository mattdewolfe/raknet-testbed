#include "XMLScriptManager.h"

XMLScriptManager::XMLScriptManager()
{

}

std::string XMLScriptManager::GetQuestionCardText(int _referenceNumber)
{
	std::string cardInfo = "q";
	cardInfo += std::to_string(_referenceNumber);
	const char *cstr = cardInfo.c_str();
	return GetStringVariableFromScript("questions", cstr);
}

std::string XMLScriptManager::GetAnswerCardText(int _referenceNumber)
{
	std::string cardInfo = "a";
	cardInfo += std::to_string(_referenceNumber);
	const char *cstr = cardInfo.c_str();
	return GetStringVariableFromScript("answers", cstr);
}

float XMLScriptManager::GetFloatVariableFromScript(char* _node, const char* _name)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	char* ret = targetAttr->value();
	float total = 0.0f;
	bool afterDecimal = false;
	for (int i = 0; ret[i] != '\0'; i++)
	{
		if (ret[i] == '.')
		{
			// If we find a second decimal place, break and ignore further values
			if (afterDecimal == true)
			{
				break;
			}
			afterDecimal = true;
		}
		else
		{
			if (afterDecimal == false)
				total += (float)(ret[i] - '0');
			else
				total += (float)(ret[i] - '0')/10;
		}
	}
	return total;
}

int XMLScriptManager::GetIntVariableFromScript(char* _node, const char* _name)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	int ret = std::stoi(targetAttr->value());
	return ret;
}

bool XMLScriptManager::GetBoolVariableFromScript(char* _node, const char* _name)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	char* attr = targetAttr->value();
	if (attr[0] == 't' || attr[0] == 'T')
	{ 
		return true;
	}
	else
	{
		return false;
	}
}

std::string XMLScriptManager::GetStringVariableFromScript(char* _node, const char* _name)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	std::string ret = targetAttr->value();
	return ret;
}

bool XMLScriptManager::Init()
{
	LoadScript("xml\\CardsAgainstHumanity.xml");
	return true;
}

bool XMLScriptManager::LoadScript(const char* _fileName)
{
	xmlFile = new rapidxml::file<>(_fileName);
	if (xmlFile != nullptr)
	{
		doc.parse<0>(xmlFile->data());
	}
	return false;
}

void XMLScriptManager::Update(float _dt)
{

}

void XMLScriptManager::Shutdown()
{
	doc.clear();

	delete xmlFile;
	xmlFile = nullptr;
}

// Global print function for string output
void PrintString(std::string &str)
{
	std::cout << str;
}


XMLScriptManager::~XMLScriptManager()
{

}
