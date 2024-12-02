#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "labels.h"

#include "utils.h"

KIMLCOMPILEERRORCODES LabelTable::AddLabel(KIMLCSTRING label)
{
	std::string s(label);
	tabletype::iterator it = this->table.find(s);

	if (it != this->table.end())
		return KCE_DUPLICATE_LABEL;

	this->table[s] = 0;
	return KCE_NONE;
}

void LabelTable::MarkLabel(KIMLCSTRING label, KIMLUINT position)
{
	std::string s(label);
	tabletype::iterator it = this->table.find(s);

	if (it != this->table.end())
		it->second = position;
}

KIMLCOMPILEERRORCODES LabelTable::AddFixup(KIMLCSTRING label, KIMLUINT position)
{
	std::string s(label);
	tabletype::iterator it = this->table.find(s);

	if (it == this->table.end())
		return KCE_UNDECLARED_LABEL;

	Fixup fixup(&(it->second), position);
	this->fixups.push_back(fixup);
	return KCE_NONE;
}

KIMLCOMPILEERRORCODES LabelTable::HasLabel(KIMLCSTRING ident)
{
	tabletype::const_iterator it = this->table.find(ident);

	if (it == this->table.end())
		return KCE_UNDECLARED_LABEL;

	return KCE_NONE;
}

void LabelTable::Finish(KIMLBYTE *codeStream)
{
	for (std::vector<Fixup>::const_iterator it = this->fixups.begin();
		it != this->fixups.end(); ++it)
	{
		Fixup fixup = *it;
		KIMLUINT *labelPosition = fixup.labelPosition;
		KIMLUINT branchingPosition = fixup.branchingPosition;

		kimlSerialize(codeStream + branchingPosition, *labelPosition);
	}
}

void LabelTable::CleanUp()
{
	this->table.clear();
}
