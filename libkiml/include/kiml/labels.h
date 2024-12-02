#ifndef __LABELS_H__
#define __LABELS_H__

#include "kiml.h"

#include <map>
#include <vector>
#include <iostream>
#include <string>

class LabelTable
{
public:
	struct Fixup
	{
		KIMLUINT *labelPosition;
		KIMLUINT branchingPosition;

		Fixup(KIMLUINT *labelPosition, KIMLUINT branchingPosition)
			: labelPosition(labelPosition), branchingPosition(branchingPosition) { }
	};

private:
	typedef std::map<std::string, KIMLUINT> tabletype;
	
private:
	tabletype table;
	std::vector<Fixup> fixups;

public:
	KIMLCOMPILEERRORCODES AddLabel(KIMLCSTRING label);
	void MarkLabel(KIMLCSTRING label, KIMLUINT position);
	KIMLCOMPILEERRORCODES AddFixup(KIMLCSTRING label, KIMLUINT position);
	KIMLCOMPILEERRORCODES HasLabel(KIMLCSTRING ident);
	void Finish(KIMLBYTE *codeStream);
	void CleanUp(void);

#if _DEBUG
	void DumpLabels()
	{
		std::cout << "Labels:\n";
		tabletype::iterator it = this->table.begin();
		for (; it != this->table.end(); ++it)
			std::cout << it->first.c_str() << " : " << it->second << std::endl;
	}
#endif
};

#endif//__LABELS_H__
