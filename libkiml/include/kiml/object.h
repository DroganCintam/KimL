#ifndef __OBJECT_H__
#define __OBJECT_H__

#include "kiml.h"
#include "utils.h"
#include "datatypes.h"

#include <string.h>
#include <math.h>
#include <sstream>

class KIML_API Object
{
private:
	KIMLTYPES datatype;
	union
	{
		KIMLBOOL    bvalue;
		KIMLINT     ivalue;
		KIMLFLOAT   fvalue;
		KIMLCSTRING svalue;
		KIMLOBJECT  ovalue;
		KIMLNINT	nivalue;
	};

public:
	Object(void)
		: datatype(KIML_INT), ivalue(0) { }

	Object(const Object &src)
	{
		memcpy(this, (void *)&src, sizeof(Object));
		if (src.datatype == KIML_STRING)
			this->svalue = kimlstrdup(src.svalue);
	}

	Object(Object &&src)
	{
		memcpy(this, (void *)&src, sizeof(Object));
		src.svalue = nullptr;
	}

	Object(KIMLTYPES t)
	{
		memset(this, 0, sizeof(Object));
		this->datatype = t;
		if (t == KIML_STRING)
		{
			KIMLSTRING s = new KIMLCHAR[1];
			s[0] = 0;
			this->svalue = s;
		}
	}

	Object(bool v)
		: datatype(KIML_BOOL), bvalue(v ? TRUE : FALSE) { }

	Object(KIMLINT v)
		: datatype(KIML_INT), ivalue(v) { }

	Object(KIMLFLOAT v)
		: datatype(KIML_REAL), fvalue(v) { }

	Object(KIMLCSTRING v)
		: datatype(KIML_STRING), svalue(kimlstrdup(v)) { }

	Object(KIMLOBJECT v)
		: datatype(KIML_OBJECT), ovalue(v) { }

	~Object(void)
	{
		if (this->datatype == KIML_STRING && this->svalue)
		{
			delete []this->svalue;
			this->svalue = nullptr;
		}
	}

	Object & operator=(const Object &src)
	{
		if (this != &src)
		{
			if (this->datatype == KIML_STRING && this->svalue)
				delete []this->svalue;

			memcpy(this, (void *)&src, sizeof(Object));
			if (src.datatype == KIML_STRING)
				this->svalue = kimlstrdup(src.svalue);
		}

		return *this;
	}

	Object & operator=(Object &&src)
	{
		if (this != &src)
		{
			if (this->datatype == KIML_STRING && this->svalue)
				delete[]this->svalue;

			memcpy(this, (void *)&src, sizeof(Object));
			src.svalue = nullptr;
		}

		return *this;
	}

	Object & operator=(bool val)
	{
		if (this->datatype == KIML_STRING && this->svalue)
			delete []this->svalue;
		
		this->datatype = KIML_BOOL;
		this->bvalue = val ? TRUE : FALSE;
		
		return *this;
	}

	Object & operator=(KIMLINT val)
	{
		if (this->datatype == KIML_STRING && this->svalue)
			delete []this->svalue;
		
		this->datatype = KIML_INT;
		this->ivalue = val;
		
		return *this;
	}

	Object & operator=(KIMLFLOAT val)
	{
		if (this->datatype == KIML_STRING && this->svalue)
			delete []this->svalue;
		
		this->datatype = KIML_REAL;
		this->fvalue = val;
		
		return *this;
	}

	Object & operator=(KIMLCSTRING val)
	{
		if (this->datatype == KIML_STRING && this->svalue)
			delete []this->svalue;
		
		this->datatype = KIML_STRING;
		this->svalue = kimlstrdup(val);
		
		return *this;
	}

	Object & operator=(KIMLOBJECT val)
	{
		if (this->datatype == KIML_STRING && this->svalue)
			delete []this->svalue;

		this->datatype = KIML_OBJECT;
		this->ovalue = val;

		return *this;
	}

	void From(const KIMLVALUE &value)
	{
		if (datatype == KIML_STRING)
			delete []svalue;

		datatype = value.type;
		ovalue = value.v.o;
		if (datatype == KIML_STRING)
			svalue = kimlstrdup(svalue);
	}

	void To(KIMLVALUE &value)
	{
		value.type = datatype;
		value.v.o = ovalue;
		if (datatype == KIML_STRING)
			value.v.s = kimlstrdup(svalue);
	}

	KIMLTYPES getDataType(void) const { return datatype; }

	KIMLBOOL getBool(void) const
	{
		switch (this->datatype)
		{
		case KIML_BOOL:
			return bvalue;
		case KIML_INT:
			return ivalue != 0 ? TRUE : FALSE;
		case KIML_REAL:
			return fvalue != 0 ? TRUE : FALSE;
		case KIML_STRING:
			return svalue[0] == 0 ? FALSE : (strlen(svalue) != 0 ? FALSE : TRUE);
		case KIML_OBJECT:
			return ovalue != nullptr;
		default:
			return FALSE;
		}
	}

	KIMLINT getInt(void) const
	{
		switch (this->datatype)
		{
		case KIML_BOOL:
			return (KIMLINT)bvalue;
		case KIML_INT:
			return ivalue;
		case KIML_REAL:
			return (KIMLINT)fvalue;
		case KIML_STRING:
			KIMLINT result;
			{
				std::istringstream is(svalue);
				if (!(is >> result))
					result = (KIMLINT)0;
			}
			return result;
		case KIML_OBJECT:
			return 0;
		default:
			return 0;
		}
	}

	KIMLFLOAT getFloat(void) const
	{
		switch (this->datatype)
		{
		case KIML_BOOL:
			return (KIMLFLOAT)bvalue;
		case KIML_INT:
			return (KIMLFLOAT)ivalue;
		case KIML_REAL:
			return fvalue;
		case KIML_STRING:
			KIMLFLOAT result;
			{
				std::istringstream is(svalue);
				if (!(is >> result))
					result = (KIMLFLOAT)0;
			}
			return result;
		default:
			return 0;
		}
	}

	KIMLCSTRING getString(void) const
	{
		KIMLCSTRING result;
		switch (this->datatype)
		{
		case KIML_BOOL:
			result = kimlstrdup(bvalue ? "true" : "false");
			break;
		case KIML_INT:
			{
				std::ostringstream os;
				os << ivalue;
				result = kimlstrdup(os.str().c_str());
			}
			break;
		case KIML_REAL:
			{
				std::ostringstream os;
				os << fvalue;
				result = kimlstrdup(os.str().c_str());
			}
			break;
		case KIML_STRING:
			result = kimlstrdup(svalue);
			break;
		case KIML_OBJECT:
			{
				if (ovalue == nullptr)
					result = kimlstrdup("null");
				else
					result = kimlstrdup("(object)");
				break;
			}
		default:
			result = nullptr;
			break;
		}
		return result;
	}

	KIMLOBJECT getObject(void) const
	{
		switch (this->datatype)
		{
		case KIML_INT:
			return (KIMLOBJECT)nivalue;
		case KIML_STRING:
			return (KIMLOBJECT)svalue;
		case KIML_OBJECT:
			return ovalue;
		default:
			return nullptr;
		}
	}

	Object operator +(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() + rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() + rhs.getFloat());
		case KIML_STRING:
			return Object::Concat(*this, rhs);
		default:
			return Object((KIMLINT)0);
		}
	}

	Object operator -(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() - rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() - rhs.getFloat());
		default:
			return Object((KIMLINT)0);
		}
	}

	Object operator -() const
	{
		switch (this->datatype)
		{
		case KIML_INT:
			return Object(-this->ivalue);
		case KIML_REAL:
			return Object(-this->fvalue);
		default:
			return Object((KIMLINT)0);
		}
	}

	Object operator *(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() * rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() * rhs.getFloat());
		default:
			return Object((KIMLINT)0);
		}
	}

	Object operator /(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() / rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() / rhs.getFloat());
		default:
			return Object((KIMLINT)0);
		}
	}

	Object operator %(const Object &rhs) const
	{
		// ASSUMED: checked for string type already

		return Object(this->getInt() % rhs.getInt());
	}

	Object operator ==(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_BOOL:
			return Object(this->getBool() == rhs.getBool());
		case KIML_INT:
			return Object(this->getInt() == rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() == rhs.getFloat());
		case KIML_OBJECT:
			return Object(this->getObject() == rhs.getObject());
		default:
			{
				KIMLCSTRING s1 = this->getString();
				KIMLCSTRING s2 = rhs.getString();
				KIMLBOOL result = strcmp(s1, s2) == 0;
				delete []s1;
				delete []s2;
				return Object(result);
			}
		}
	}

	Object operator !=(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_BOOL:
			return Object(this->getBool() != rhs.getBool());
		case KIML_INT:
			return Object(this->getInt() != rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() != rhs.getFloat());
		case KIML_OBJECT:
			return Object(this->getObject() != rhs.getObject());
		default:
			{
				KIMLCSTRING s1 = this->getString();
				KIMLCSTRING s2 = rhs.getString();
				bool result = strcmp(s1, s2) != 0;
				delete []s1;
				delete []s2;
				return Object(result);
			}
		}
	}

	Object operator <(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() < rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() < rhs.getFloat());
		case KIML_OBJECT:
			return Object((intptr_t)this->getObject() < (intptr_t)rhs.getObject());
		default:
			return Object(false);
		}
	}

	Object operator <=(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() <= rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() <= rhs.getFloat());
		case KIML_OBJECT:
			return Object((intptr_t)this->getObject() <= (intptr_t)rhs.getObject());
		default:
			return Object(false);
		}
	}

	Object operator >(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() > rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() > rhs.getFloat());
		case KIML_OBJECT:
			return Object((intptr_t)this->getObject() > (intptr_t)rhs.getObject());
		default:
			return Object(false);
		}
	}

	Object operator >=(const Object &rhs) const
	{
		switch (HigherType(this->datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object(this->getInt() >= rhs.getInt());
		case KIML_REAL:
			return Object(this->getFloat() >= rhs.getFloat());
		case KIML_OBJECT:
			return Object((intptr_t)this->getObject() >= (intptr_t)rhs.getObject());
		default:
			return Object(false);
		}
	}

	Object operator &&(const Object &rhs) const
	{
		return Object(this->getBool() && rhs.getBool());
	}

	Object operator ||(const Object &rhs) const
	{
		return Object(this->getBool() || rhs.getBool());
	}

	Object operator ^(const Object &rhs) const
	{
		return Object(this->getBool() != rhs.getBool());
	}

	Object operator !() const
	{
		return Object(this->getBool() ? false : true);
	}

	static Object Pow(const Object &lhs, const Object &rhs)
	{
		switch (HigherType(lhs.datatype, rhs.datatype))
		{
		case KIML_INT:
			return Object((KIMLINT)pow((double)lhs.getInt(), (double)rhs.getInt()));
		case KIML_REAL:
			return Object((KIMLFLOAT)pow((double)lhs.getFloat(), (double)rhs.getFloat()));
		default:
			return Object((KIMLINT)0);
		}
	}

	static Object Concat(const Object &lhs, const Object &rhs)
	{
		KIMLCSTRING s1 = lhs.getString(), s2 = rhs.getString();
		KIMLUINT len1 = strlen(s1), len2 = strlen(s2);

		KIMLSTRING s = new KIMLCHAR[len1 + len2 + 1];
		strncpy(s, s1, len1);
		strncpy(s + len1, s2, len2);
		s[len1+len2] = 0;

		Object obj(s);

		delete []s1;
		delete []s2;
		delete []s;

		return obj;
	}
};

#endif//__OBJECT_H__
