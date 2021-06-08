#include "sc_tool/utils/NameGenerator.h"
#include "sc_tool/utils/VerilogKeywords.h"
#include <iostream>

using std::cout; using std::endl;

std::string UniqueNamesGenerator::getUniqueName(const std::string &suggestedName,
                                                bool checkLocalNames)
{
    std::string genName = suggestedName;
    
    // If given name is Verilog keyword add some suffix
    bool isKeyword = sc::VerilogKeywords::check(genName);
    if (isKeyword) {
        genName += "_v";
    }

    int postfix = 0;

    // Avoid register name conflict with local name
    while (takenNames.count(genName) || 
           (checkLocalNames && localNames.count(genName))) 
    {
        genName = suggestedName + /*"_" +*/ std::to_string(postfix);
        ++postfix;
    }

    if (isKeyword || postfix) {
        changedNames.insert(suggestedName);
    }
    
    takenNames.insert(genName);

    //std::cout << "getUniqueName " << suggestedName << " " << genName << std::endl;
    return genName;
}

bool UniqueNamesGenerator::isTaken(const std::string &name, bool checkLocalNames)
{
    return (takenNames.count(name) || (checkLocalNames && localNames.count(name)));
}

bool UniqueNamesGenerator::isChanged(const std::string &name)
{
    //std::cout << " Check name " << name << " changed : " << changedNames.count(name) << std::endl;
    return changedNames.count(name) != 0;
}

void UniqueNamesGenerator::print() const
{
    cout << "namesGenerator size" << takenNames.size() << endl;
    for (const std::string& name : takenNames) {
        cout << "  " << name << endl;
    }
}
