#include "XMLScriptManager.h"

XMLScriptManager::XMLScriptManager()
{

}

float XMLScriptManager::GetVariableFromScript(char* _node, char* _name, float* var)
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
				DBOUT("XML: Read float: Terminated early due to second decimal.");
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
	DBOUT("XML: Read float: " << total);
	return total;
}

int XMLScriptManager::GetVariableFromScript(char* _node, char* _name, int* var)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	int ret = std::stoi(targetAttr->value());
	DBOUT("XML: Read int: " << ret);
	return ret;
}

bool XMLScriptManager::GetVariableFromScript(char* _node, char* _name, bool* var)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	char* attr = targetAttr->value();
	if (attr[0] == 't' || attr[0] == 'T')
	{ 
		DBOUT("XML: Read bool: true");
		return true;
	}
	else
	{
		DBOUT("XML: Read bool: false");
		return false;
	}
}

std::string XMLScriptManager::GetVariableFromScript(char* _node, char* _name, std::string* var)
{
	rapidxml::xml_node<>* parent = doc.first_node();
	rapidxml::xml_node<>* targetNode = parent->first_node(_node);
	rapidxml::xml_node<>* targetAttr = targetNode->first_node(_name);
	std::string ret = targetAttr->value();
	DBOUT("XML: Read string: " << targetAttr->value());
	return ret;
}

bool XMLScriptManager::Init()
{
	LoadScript("scripts\\TestXML.xml");
	return true;
}

bool XMLScriptManager::LoadScript(const char* _fileName)
{
	xmlFile = new rapidxml::file<>(_fileName);
	if (xmlFile != nullptr)
	{
		doc.parse<0>(xmlFile->data());
	}
	DBOUT( "XML: Loaded " << doc.first_node()->name() << " script." );
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
