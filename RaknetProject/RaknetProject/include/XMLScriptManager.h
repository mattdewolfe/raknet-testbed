#ifndef XML_SCRIPT_MANAGER_H
#define XML_SCRIPT_MANAGER_H

#include <assert.h>
#include <Windows.h>
#include <iostream>
#include <sstream> 
#include <string>
#include "rapidXML\rapidxml.hpp"
#include "rapidXML\rapidxml_iterators.hpp"
#include "rapidXML\rapidxml_print.hpp"
#include "rapidXML\rapidxml_utils.hpp"

// Macro for tracing event data names into output window
#define DBOUT( s )            \
{                             \
   std::wostringstream os_;    \
   os_ << s << "\n";                   \
   OutputDebugStringW( os_.str().c_str() );  \
}

class XMLScriptManager
{
public:
	XMLScriptManager();
	~XMLScriptManager();

	bool Init();
	void Update(float _dt);
	void Shutdown();
	bool LoadScript(const char* _fileName);

	// Get a variable by passing in the variable name in script
	// @Parem - node to look under, attribute name, variable type to return
	float GetFloatVariableFromScript(char* _node, char* _name);
	int GetIntVariableFromScript(char* _node, char* _name);
	bool GetBoolVariableFromScript(char* _node, char* _name);
	std::string GetStringVariableFromScript(char* _node, char* _name);
	
private:
	/* For further functionilty, we could look into changing the variables stored in our scripts
	* from inside our engine, add additional component parameters to be created via script, 
	* or add in the ability to run functions based entirely on scripting 
	void CreateComponentFromScript(char* _name, ICompoment* comp);
	float SetVariableInScript(char* _name, float &var);
	int SetVariableInScript(char* _name, int &var);
	bool SetVariableInScript(char* _name, bool &var);
	*/

	rapidxml::xml_document<> doc;
	rapidxml::file<>* xmlFile;
};

#endif