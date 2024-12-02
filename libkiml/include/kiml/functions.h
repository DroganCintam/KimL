#ifndef __FUNCTIONS_H__
#define __FUNCTIONS_H__

#include "kiml.h"

#include <map>
#include <vector>
#include <string>
#include <string.h>
#include <stdarg.h>

struct FunctionInfo
{
	KIMLUINT nameHash;
	KIMLTYPES returnType;
	KIMLUINT paramCount;
	KIMLTYPES *paramTypes;

	FunctionInfo(void)
		: nameHash(0), returnType(KIML_NOTYPE),
		paramCount(0), paramTypes(nullptr) { }

	FunctionInfo(const FunctionInfo &src)
		: nameHash(src.nameHash), returnType(src.returnType),
		paramCount(src.paramCount), paramTypes(nullptr)
	{
		if (src.paramTypes)
		{
			paramTypes = new KIMLTYPES[paramCount];
			if (paramTypes)
				memcpy(paramTypes, src.paramTypes, sizeof(KIMLTYPES) * paramCount);
		}
	}

	FunctionInfo(FunctionInfo &&src)
		: nameHash(src.nameHash), returnType(src.returnType),
		paramCount(src.paramCount), paramTypes(nullptr)
	{
		if (src.paramTypes)
		{
			paramTypes = src.paramTypes;
			src.paramTypes = nullptr;
		}
	}

	FunctionInfo(KIMLCSTRING name, KIMLTYPES returnType, ...)
	{
		this->nameHash = kimlHashName(name);
		this->returnType = returnType;

		std::vector<KIMLTYPES> params;

		va_list ap;
		va_start(ap, returnType);

		KIMLTYPES paramt = (KIMLTYPES)va_arg(ap, int);
		while (paramt != KIML_NOTYPE)
		{
			params.push_back(paramt);
			paramt = (KIMLTYPES)va_arg(ap, int);
		}

		va_end(ap);

		this->setParams(params);
	}

	~FunctionInfo(void)
	{
		if (paramTypes)
		{
			delete []paramTypes;
			paramTypes = nullptr;
		}
	}

	FunctionInfo & operator=(const FunctionInfo &rhs)
	{
		if (this != &rhs)
		{
			nameHash = rhs.nameHash;
			returnType = rhs.returnType;
			paramCount = rhs.paramCount;

			if (paramTypes)
				delete []paramTypes;
			paramTypes = nullptr;
			
			if (rhs.paramTypes)
			{
				paramTypes = new KIMLTYPES[paramCount];
				if (paramTypes)
					memcpy(paramTypes, rhs.paramTypes, sizeof(KIMLTYPES) * paramCount);
			}
		}

		return *this;
	}

	FunctionInfo & operator=(FunctionInfo &&rhs)
	{
		if (this != &rhs)
		{
			nameHash = rhs.nameHash;
			returnType = rhs.returnType;
			paramCount = rhs.paramCount;

			if (paramTypes)
				delete []paramTypes;
			paramTypes = nullptr;
			
			if (rhs.paramTypes)
			{
				paramTypes = rhs.paramTypes;
				rhs.paramTypes = nullptr;
			}
		}

		return *this;
	}

	void setParams(const std::vector<KIMLTYPES> &paramTypes)
	{
		this->paramCount = paramTypes.size();
		this->paramTypes = new KIMLTYPES[this->paramCount];
		if (this->paramCount)
			memcpy(this->paramTypes, (void *)&paramTypes[0], sizeof(KIMLTYPES) * this->paramCount);
	}
};

extern KIMLCSTRING builtinFunctionNames[];
extern FunctionInfo builtinFunctionInfos[];

class FunctionTable
{
private:
	typedef std::map<std::string, FunctionInfo> tabletype;

private:
	tabletype table;

public:
	KIMLCOMPILEERRORCODES AddFunction(KIMLCSTRING name, const FunctionInfo &funcInfo);
	KIMLCOMPILEERRORCODES GetFunction(KIMLCSTRING name, FunctionInfo &funcInfo);

	static const FunctionInfo * GetBuiltinFunctionInfo(KIMLCSTRING name, KIMLUINT &index);

	void CleanUp(void);
};

#endif//__FUNCTIONS_H__
