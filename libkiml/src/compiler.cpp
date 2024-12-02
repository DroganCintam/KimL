#ifdef _MSC_VER
#include "stdafx.h"
#endif

#include "compiler.h"
#include "errmsg.h"
#include "datatypes.h"

#define safeDelete(p) do { if (p) { delete p; p = nullptr; } } while(false)
#define safeDeleteArray(a) do { if (a) { delete []a; a = nullptr; } } while(false)

#define raiseCompileError(_err, _ln) \
	do { \
		KIMLCOMPILEERRORINFO e = { _err, _ln }; \
		this->errors.push_back(e); \
		return false; \
	} while (0)

#define raiseCompileErrorNoReturn(_err, _ln) \
	do { \
		KIMLCOMPILEERRORINFO e = { _err, _ln }; \
		this->errors.push_back(e); \
	} while (0)


// ========== parser & lexer =============
extern FILE *yyin;
extern AstProgram *root;
extern KIMLUINT nSyntaxErrors;
extern KIMLCSTRING szFileName;
extern int yylineno; //kiml.lexer.cpp

extern int yyparse(void);
// =======================================

Compiler::Compiler(void)
	: labelTable(nullptr), symbolTable(nullptr), constTable(nullptr), funcTable(nullptr),
	byteCodeStream(nullptr), programSize(0), line(0),
	stringPool(nullptr), stringPoolSize(0),
	hasDebugInfo(false), fp(nullptr), inIncludeMode(false)
{
	this->labelTable = new LabelTable();
	this->symbolTable = new SymbolTable();
	this->constTable = new ConstTable();
	this->funcTable = new FunctionTable();
	this->byteCodeStream = new ByteCodeStream();
	this->stringPool = new StringPool();
}

Compiler::~Compiler(void)
{
	CleanUpAll();
	
	safeDelete(labelTable);
	safeDelete(symbolTable);
	safeDelete(constTable);
	safeDelete(funcTable);
	safeDelete(byteCodeStream);
	safeDelete(stringPool);
}

bool Compiler::CompileInclude(KIMLCSTRING filename, FILE *fp)
{
	szFileName = filename;
	this->fp = fp;
	this->hasDebugInfo = false;
	this->inIncludeMode = true;

	yyin = this->fp;
	
	if (yyparse() || nSyntaxErrors)
		return false;

	if (!this->StartCompileInclude())
		return false;

	return true;
}

bool Compiler::Compile(KIMLCSTRING filename, FILE *fp, bool hasDebugInfo)
{
	szFileName = filename;
	this->fp = fp;
	this->hasDebugInfo = hasDebugInfo;
	this->inIncludeMode = false;

	yyin = this->fp;

	if (yyparse() || nSyntaxErrors)
		return false;

	if (!this->StartCompile())
		return false;

	return true;
}

void Compiler::CleanUpAll()
{
	labelTable->CleanUp();
	symbolTable->CleanUp();
	constTable->CleanUp();
	funcTable->CleanUp();
}

bool Compiler::Emit(OpCodes opcode)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;
	bytecode << opcode;
	return true;
}

bool Compiler::Emit(OpCodes opcode, KIMLINT value)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;
	bytecode << opcode << value;
	return true;
}

bool Compiler::Emit(OpCodes opcode, KIMLUINT value)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;
	bytecode << opcode << value;
	return true;
}

bool Compiler::Emit(OpCodes opcode, KIMLFLOAT value)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;
	bytecode << opcode << value;
	return true;
}

bool Compiler::Emit(OpCodes opcode, KIMLCSTRING value)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;

	if (value[0] == 0)
	{
		return false;
	}
	else
	{
		this->stringPool->AddString(value);
		bytecode << opcode << this->stringPool->GetStringAddress(value);
		return true;
	}
}

bool Compiler::EmitJump(OpCodes opcode, KIMLCSTRING label)
{
	if (opcode != op_jmp && opcode != op_cjmp)
		return false;

	ByteCodeStream &bytecode = *this->byteCodeStream;

	bytecode << opcode;

	this->labelTable->AddFixup(label, bytecode.GetStreamSize());
	bytecode << (KIMLUINT)0;
	
	return true;
}

bool Compiler::EmitFuncDecl(KIMLCSTRING funcName)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;

	bytecode << op_fdecl << kimlHashName(funcName);

	this->labelTable->AddFixup(funcName, bytecode.GetStreamSize());
	bytecode << (KIMLUINT)0;

	return true;
}

bool Compiler::AddLabel(KIMLCSTRING label)
{
	this->labelTable->AddLabel(label);
	return true;
}

bool Compiler::MarkLabel(KIMLCSTRING label)
{
	this->labelTable->MarkLabel(label, this->byteCodeStream->GetStreamSize());
	return true;
}

bool Compiler::Bake(void)
{
	this->labelTable->Finish(this->byteCodeStream->GetStream());
	return true;
}

bool Compiler::StartCompileInclude(void)
{
	if (!this->constTable->HasConst("KIMLVER"))
	{
		Object obj(KIML_VERSION);
		this->constTable->AddConst("KIMLVER", obj);
	}

	errors.clear();
	nSyntaxErrors = 0;
	yylineno = 1;

	if (!this->AnalyseSemanticsInclude(root))
		return false;

	return true;
}

bool Compiler::StartCompile(void)
{
	if (!this->constTable->HasConst("KIMLVER"))
	{
		Object obj(KIML_VERSION);
		this->constTable->AddConst("KIMLVER", obj);
	}

	errors.clear();
	nSyntaxErrors = 0;
	yylineno = 1;

	if (!this->AnalyseSemantics(root))
		return false;

	this->Emit(root);

	*this->byteCodeStream << op_hlt;

	return true;
}

bool Compiler::AddFunction(KIMLCSTRING name, KIMLTYPES returnType, const std::vector<KIMLTYPES> &paramTypes)
{
	FunctionInfo fi;
	fi.returnType = returnType;
	fi.nameHash = kimlHashName(name);
	fi.setParams(paramTypes);

	KIMLCOMPILEERRORCODES err = this->funcTable->AddFunction(name, fi);
	if (err)
		raiseCompileError(err, line);

	return true;
}

bool Compiler::AddVariable(KIMLCSTRING name, KIMLTYPES type)
{
	KIMLCOMPILEERRORCODES err = this->symbolTable->AddSymbol(name, type);
	if (err)
		raiseCompileError(err, line);

	return true;
}


bool Compiler::AddConst(KIMLCSTRING name, const Object &value)
{
	KIMLCOMPILEERRORCODES err = this->constTable->AddConst(name, value);
	if (err)
		raiseCompileError(err, line);

	return true;
}

void Compiler::Emit(AstProgram *program)
{
	std::vector<AstStatement *> &statements = program->getStatements()->getStatements();
	
	for (auto it = statements.begin(); it != statements.end(); ++it)
	{
		AstStatement *stmt = *it;
		if (!stmt || stmt->getType() == EMPTYLINE)
			continue;

		this->Emit(stmt);
	}

	this->labelTable->Finish(this->byteCodeStream->GetStream());
}

void Compiler::Emit(AstStatement *statement)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;

	if (this->hasDebugInfo)
		bytecode << op_trace << (KIMLUINT)statement->getLocation().first_line;

	switch (statement->getType())
	{
	case IOIN:
		{
			auto ioin = static_cast<AstIoIn *>(statement);
			if (ioin->getInputIdent())
			{
				KIMLUINT h = kimlHashName(ioin->getInputIdent());
				KIMLTYPES t;
				this->symbolTable->GetSymbolType(ioin->getInputIdent(), t);
				switch (t)
				{
				case KIML_BOOL:
					bytecode << op_in << op_cvb << op_stvar << h;
					break;
				case KIML_INT:
					bytecode << op_in << op_cvin << op_stvar << h;
					break;
				case KIML_REAL:
					bytecode << op_in << op_cvrn << op_stvar << h;
					break;
				case KIML_STRING:
					bytecode << op_in << op_stvar << h;
					break;
				case KIML_OBJECT:
					bytecode << op_in << op_cvo << op_stvar << h;
					break;
				}
			}
			else
			{
				switch (ioin->getInputType())
				{
				case KIML_BOOL:
					bytecode << op_in << op_cvb << op_push;
					break;
				case KIML_INT:
					bytecode << op_in << op_cvin << op_push;
					break;
				case KIML_REAL:
					bytecode << op_in << op_cvrn << op_push;
					break;
				case KIML_STRING:
					bytecode << op_in << op_push;
					break;
				case KIML_OBJECT:
					bytecode << op_in << op_cvo << op_push;
					break;
				}
			}
		}
		break;

	case IOOUT:
		{
			auto ioout = static_cast<AstIoOut *>(statement);
			this->Emit(ioout->getValue());
			bytecode << op_out;
		}
		break;

	case VARDECL:
		{
			auto vardecl = static_cast<AstVarDecl *>(statement);
			this->symbolTable->AddSymbol(vardecl->getVarName(), vardecl->getVarType());

			auto val = vardecl->getDefaultValue();
			if (val)
			{
				this->Emit(val);
				if (TypeMatchTable[vardecl->getVarType()][val->getDataType()] == K_TYPE_CAST)
				{
					switch (vardecl->getVarType())
					{
					case KIML_BOOL:
						bytecode << op_cvb;
						break;
					case KIML_INT:
						bytecode << op_cvi;
						break;
					case KIML_REAL:
						bytecode << op_cvr;
						break;
					case KIML_STRING:
						bytecode << op_cvs;
						break;
					case KIML_OBJECT:
						bytecode << op_cvo;
						break;
					}
				}
			}
			else
			{
				switch (vardecl->getVarType())
				{
					case KIML_BOOL:
						bytecode << op_ldfalse;
						break;
					case KIML_INT:
						bytecode << op_ldi << (KIMLINT)0;
						break;
					case KIML_REAL:
						bytecode << op_ldr << (KIMLFLOAT)0;
						break;
					case KIML_STRING:
						bytecode << op_ldes;
						break;
					case KIML_OBJECT:
						bytecode << op_ldnull;
						break;
				}
			}

			bytecode << op_new << kimlHashName(vardecl->getVarName());
		}
		break;

	case VARSET:
		{
			auto varset = static_cast<AstVarSet *>(statement);
			KIMLTYPES t;
			this->symbolTable->GetSymbolType(varset->getVarName(), t);

			auto val = varset->getValue();
			this->Emit(val);

			if (TypeMatchTable[t][val->getDataType()] == K_TYPE_CAST)
			{
				switch (t)
				{
				case KIML_BOOL:
					bytecode << op_cvb;
					break;
				case KIML_INT:
					bytecode << op_cvi;
					break;
				case KIML_REAL:
					bytecode << op_cvr;
					break;
				case KIML_STRING:
					bytecode << op_cvs;
					break;
				case KIML_OBJECT:
					bytecode << op_cvo;
					break;
				}
			}

			bytecode << op_stvar << kimlHashName(varset->getVarName());
		}
		break;

	case VARDEL:
		{
			auto vardel = static_cast<AstVarDel *>(statement);
			this->symbolTable->RemoveSymbol(vardel->getVarName());
			bytecode << op_free << kimlHashName(vardel->getVarName());
		}
		break;

	case STACKPUSH:
		{
			auto stackpush = static_cast<AstStackPush *>(statement);
			this->Emit(stackpush->getValue());
			bytecode << op_push;
		}
		break;

	case STACKPOP:
		{
			auto stackpop = static_cast<AstStackPop *>(statement);
			if (stackpop->getVarName())
			{
				KIMLTYPES t;
				this->symbolTable->GetSymbolType(stackpop->getVarName(), t);
				switch (t)
				{
				case KIML_BOOL:
					bytecode << op_smv << op_cvb;
					break;
				case KIML_INT:
					bytecode << op_smv << op_cvi;
					break;
				case KIML_REAL:
					bytecode << op_smv << op_cvr;
					break;
				case KIML_STRING:
					bytecode << op_smv << op_cvs;
					break;
				case KIML_OBJECT:
					bytecode << op_smv << op_cvo;
					break;
				}
			}
			else
			{
				bytecode << op_pop;
			}
		}
		break;

	case STACKPEEK:
		{
			auto stackpeek = static_cast<AstStackPeek *>(statement);
			KIMLTYPES t;
			this->symbolTable->GetSymbolType(stackpeek->getVarName(), t);
			switch (t)
			{
			case KIML_BOOL:
				bytecode << op_stf << op_cvb;
				break;
			case KIML_INT:
				bytecode << op_stf << op_cvi;
				break;
			case KIML_REAL:
				bytecode << op_stf << op_cvr;
				break;
			case KIML_STRING:
				bytecode << op_stf << op_cvs;
				break;
			case KIML_OBJECT:
				bytecode << op_stf << op_cvo;
				break;
			}
		}
		break;

	case STACKSWAP:
		bytecode << op_swap;
		break;

	case STACKCLEAR:
		{
			auto stackclear = static_cast<AstStackClear *>(statement);
			if (stackclear->getCount() > 0)
				bytecode << op_sclrc << stackclear->getCount();
			else
				bytecode << op_sclr;
		}
		break;

	case STACKREAD:
		{
			auto stackread = static_cast<AstStackRead *>(statement);

			KIMLINT offset = stackread->getOffset()->getConstValue()->getInt();
			switch (offset)
			{
			case 1:
				bytecode << op_srd1;
				break;
			case 2:
				bytecode << op_srd2;
				break;
			case 3:
				bytecode << op_srd3;
				break;
			case 4:
				bytecode << op_srd4;
				break;
			case 5:
				bytecode << op_srd5;
				break;
			default:
				bytecode << op_srd << offset;
				break;
			}

			KIMLTYPES t;
			this->symbolTable->GetSymbolType(stackread->getVarName(), t);

			switch (t)
			{
			case KIML_BOOL:
				bytecode << op_cvb;
				break;
			case KIML_INT:
				bytecode << op_cvi;
				break;
			case KIML_REAL:
				bytecode << op_cvr;
				break;
			case KIML_STRING:
				bytecode << op_cvs;
				break;
			case KIML_OBJECT:
				bytecode << op_cvo;
				break;
			}

			bytecode << op_stvar << kimlHashName(stackread->getVarName());
		}
		break;

	case STACKWRITE:
		{
			auto stackwrite = static_cast<AstStackWrite *>(statement);
			this->Emit(stackwrite->getValue());

			KIMLINT offset = stackwrite->getOffset()->getConstValue()->getInt();
			switch (offset)
			{
			case 1:
				bytecode << op_swr1;
				break;
			case 2:
				bytecode << op_swr2;
				break;
			case 3:
				bytecode << op_swr3;
				break;
			case 4:
				bytecode << op_swr4;
				break;
			case 5:
				bytecode << op_swr5;
				break;
			default:
				bytecode << op_swr << offset;
				break;
			}
		}
		break;

	case TAPEREAD:
		{
			auto taperead = static_cast<AstTapeRead *>(statement);
			if (taperead->getIndex())
			{
				KIMLINT idx = taperead->getIndex()->getConstValue()->getInt();
				if (idx == 0)
					bytecode << op_trd0;
				else
					bytecode << op_trdx << idx;
			}
			else
			{
				bytecode << op_trd;
			}

			KIMLTYPES t;

			if (taperead->getDestVar())
				this->symbolTable->GetSymbolType(taperead->getDestVar(), t);
			else
				t = taperead->getDestType();

			switch (t)
			{
			case KIML_BOOL:
				bytecode << op_cvb;
				break;
			case KIML_INT:
				bytecode << op_cvi;
				break;
			case KIML_REAL:
				bytecode << op_cvr;
				break;
			case KIML_STRING:
				bytecode << op_cvs;
				break;
			case KIML_OBJECT:
				bytecode << op_cvo;
				break;
			}

			if (taperead->getDestVar())
				bytecode << op_stvar << kimlHashName(taperead->getDestVar());
			else
				bytecode << op_push;
		}
		break;

	case TAPEWRITE:
		{
			auto tapewrite = static_cast<AstTapeWrite *>(statement);
			this->Emit(tapewrite->getValue());

			if (tapewrite->getIndex())
			{
				KIMLINT idx = tapewrite->getIndex()->getConstValue()->getInt();
				if (idx == 0)
					bytecode << op_twr0;
				else
					bytecode << op_twrx << idx;
			}
			else
			{
				bytecode << op_twr;
			}
		}
		break;

	case TAPENEXT:
		bytecode << op_tnxt;
		break;

	case TAPEPREV:
		bytecode << op_tprv;
		break;

	case TAPEMOVE:
		{
			auto tapemove = static_cast<AstTapeMove *>(statement);
			this->Emit(tapemove->getValue());

			if (TypeMatchTable[KIML_INT][tapemove->getValue()->getDataType()] == K_TYPE_CAST)
				bytecode << op_cvi;

			bytecode << op_tmov;
		}
		break;

	case CTRLGOTO:
		{
			auto ctrlgoto = static_cast<AstCtrlGoto *>(statement);
			if (ctrlgoto->getCondition())
			{
				this->Emit(ctrlgoto->getCondition());
				bytecode << op_cjmp;
			}
			else
			{
				bytecode << op_jmp;
			}

			// add current code position as fixup
			// so that real label position will be
			// filled into this later.
			this->labelTable->AddFixup(ctrlgoto->getLabel(), bytecode.GetStreamSize());
			bytecode << (KIMLUINT)0;
		}
		break;

	case CTRLCALL:
		{
			auto ctrlcall = static_cast<AstCtrlCall *>(statement);
			if (ctrlcall->getCondition())
			{
				this->Emit(ctrlcall->getCondition());
				bytecode << op_ccall;
			}
			else
			{
				bytecode << op_call;
			}

			// add current code position as fixup
			// so that real label position will be
			// filled into this later.
			this->labelTable->AddFixup(ctrlcall->getLabel(), bytecode.GetStreamSize());
			bytecode << (KIMLUINT)0;
		}
		break;

	case CTRLRETURN:
		bytecode << op_ret;
		break;

	case CTRLEND:
		bytecode << op_hlt;
		break;

	case FUNCDECL:
		{
			auto funcdecl = static_cast<AstFuncDecl *>(statement);
			FunctionInfo fi;
			this->funcTable->GetFunction(funcdecl->getName(), fi);
			
			if (funcdecl->getLabel())
			{
				// instruction
				bytecode << op_fdecl << fi.nameHash;
				this->labelTable->AddFixup(funcdecl->getLabel(), bytecode.GetStreamSize());
				bytecode << (KIMLUINT)0;
			}
		}
		break;

	case FUNCCALL:
		{
			auto funccall = static_cast<AstFuncCall *>(statement);
			FunctionInfo fi;
			this->funcTable->GetFunction(funccall->getName(), fi);

			auto args = funccall->getArgs()->getExpressions();
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				this->Emit(arg);

				if (TypeMatchTable[fi.paramTypes[idx]][arg->getDataType()] == K_TYPE_CAST)
				{
					switch (fi.paramTypes[idx])
					{
					case KIML_BOOL:
						bytecode << op_cvb;
						break;
					case KIML_INT:
						bytecode << op_cvi;
						break;
					case KIML_REAL:
						bytecode << op_cvr;
						break;
					case KIML_STRING:
						bytecode << op_cvs;
						break;
					case KIML_OBJECT:
						bytecode << op_cvo;
						break;
					}
				}

				bytecode << op_push;
			}
			bytecode << op_fcall << fi.nameHash;
		}
		break;

	case EVENTHANDLE:
		{
			auto eventhandle = static_cast<AstEventHandle *>(statement);
			bytecode << op_ehnd << kimlHashName(eventhandle->getName());
			this->labelTable->AddFixup(eventhandle->getLabel(), bytecode.GetStreamSize());
			bytecode << (KIMLUINT)0;
		}
		break;

	case EVENTUNHANDLE:
		{
			auto eventunhandle = static_cast<AstEventUnhandle *>(statement);
			bytecode << op_eunhnd << kimlHashName(eventunhandle->getName());
			this->labelTable->AddFixup(eventunhandle->getLabel(), bytecode.GetStreamSize());
			bytecode << (KIMLUINT)0;
		}
		break;

	case EVENTRAISE:
		{
			auto eventraise = static_cast<AstEventRaise *>(statement);
			bytecode << op_eraise << kimlHashName(eventraise->getName());
		}
		break;

	case LABELDECL:
		this->labelTable->MarkLabel(static_cast<AstLabel *>(statement)->getName(), bytecode.GetStreamSize());
		break;
	}
}

void Compiler::Emit(AstExpression *expression)
{
	ByteCodeStream &bytecode = *this->byteCodeStream;

	if (this->FoldExpression(expression))
	{
		Object *constval = expression->getConstValue();
		switch (constval->getDataType())
		{
		case KIML_BOOL:
			bytecode << (constval->getBool() ? op_ldtrue : op_ldfalse);
			break;
		case KIML_INT:
			bytecode << op_ldi << constval->getInt();
			break;
		case KIML_REAL:
			bytecode << op_ldr << constval->getFloat();
			break;
		case KIML_STRING:
			{
				KIMLCSTRING s = constval->getString();
				if (s[0] == 0)
				{
					bytecode << op_ldes;
				}
				else
				{
					this->stringPool->AddString(s);
					bytecode << op_lds << this->stringPool->GetStringAddress(s);
				}
				delete []s;
			}
			break;
		case KIML_OBJECT:
			bytecode << op_ldnull; // as the only constval for object type is null
			break;
		}

		return; //no more things to do
	}

	switch (expression->getType())
	{
	case EXPR_IDENT:
		{
			Object obj;
			if (this->constTable->GetConst(static_cast<AstIdent *>(expression)->getIdent(), obj))
			{
				switch (obj.getDataType())
				{
				case KIML_BOOL:
					bytecode << (obj.getBool() ? op_ldtrue : op_ldfalse);
					break;
				case KIML_INT:
					bytecode << op_ldi << obj.getInt();
					break;
				case KIML_REAL:
					bytecode << op_ldr << obj.getFloat();
					break;
				case KIML_STRING:
					{
						KIMLCSTRING s = obj.getString();
						if (s[0] == 0)
						{
							bytecode << op_ldes;
						}
						else
						{
							this->stringPool->AddString(s);
							bytecode << op_lds << this->stringPool->GetStringAddress(s);
						}
						delete []s;
					}
					break;
				}
			}
			else
			{
				bytecode << op_ldvar << kimlHashName(static_cast<AstIdent *>(expression)->getIdent());
			}
		}
		break;

	case EXPR_TRUE:
		bytecode << op_ldtrue;
		break;

	case EXPR_FALSE:
		bytecode << op_ldfalse;
		break;

	case EXPR_NULL:
		bytecode << op_ldnull;
		break;

	case EXPR_INT:
		bytecode << op_ldi << static_cast<AstConstInt *>(expression)->getValue();
		break;

	case EXPR_REAL:
		bytecode << op_ldr << static_cast<AstConstReal *>(expression)->getValue();
		break;

	case EXPR_STRING:
		{
			auto litstring = static_cast<AstConstString *>(expression);
			if (litstring->getValue()[0] == 0)
			{
				bytecode << op_ldes;
			}
			else
			{
				this->stringPool->AddString(litstring->getValue());
				bytecode << op_lds << this->stringPool->GetStringAddress(litstring->getValue());
			}
		}
		break;

	case EXPR_BIN:
		{
			auto binexpr = static_cast<AstBinExpr *>(expression);
			this->Emit(binexpr->getLeft());
			this->Emit(binexpr->getRight());

			switch (binexpr->getOperator())
			{
			case BINOP_EQU:
				bytecode << op_equ;
				break;
			case BINOP_NEQ:
				bytecode << op_neq;
				break;
			case BINOP_LES:
				bytecode << op_lt;
				break;
			case BINOP_LEQ:
				bytecode << op_lte;
				break;
			case BINOP_GRT:
				bytecode << op_gt;
				break;
			case BINOP_GEQ:
				bytecode << op_gte;
				break;
			case BINOP_ADD:
				bytecode << op_add;
				break;
			case BINOP_SUB:
				bytecode << op_sub;
				break;
			case BINOP_MUL:
				bytecode << op_mul;
				break;
			case BINOP_DIV:
				bytecode << op_div;
				break;
			case BINOP_MOD:
				bytecode << op_mod;
				break;
			case BINOP_POW:
				bytecode << op_pow;
				break;
			case BINOP_CAT:
				bytecode << op_cat;
				break;
			case BINOP_AND:
				bytecode << op_and;
				break;
			case BINOP_OR:
				bytecode << op_or;
				break;
			case BINOP_XOR:
				bytecode << op_xor;
				break;
			}
		}
		break;

	case EXPR_UN:
		{
			auto unexpr = static_cast<AstUnExpr *>(expression);
			this->Emit(unexpr->getExpression());
			switch (unexpr->getOperator())
			{
			case UNOP_NEG:
				bytecode << op_neg;
				break;
			case UNOP_NOT:
				bytecode << op_not;
				break;
			}
		}
		break;

	case EXPR_FUNC:
		{
			auto exprfunc = static_cast<AstExprFunc *>(expression);
			FunctionInfo fi;

			KIMLUINT fidx;
			const FunctionInfo *builtinFI = FunctionTable::GetBuiltinFunctionInfo(exprfunc->getName(), fidx);
			if (builtinFI)
				fi = *builtinFI;
			else
				this->funcTable->GetFunction(exprfunc->getName(), fi);

			auto args = exprfunc->getArgs()->getExpressions();
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				this->Emit(arg);

				if (TypeMatchTable[fi.paramTypes[idx]][arg->getDataType()] == K_TYPE_CAST)
				{
					switch (fi.paramTypes[idx])
					{
					case KIML_BOOL:
						bytecode << op_cvb;
						break;
					case KIML_INT:
						bytecode << op_cvi;
						break;
					case KIML_REAL:
						bytecode << op_cvr;
						break;
					case KIML_STRING:
						bytecode << op_cvs;
						break;
					case KIML_OBJECT:
						bytecode << op_cvo;
						break;
					}
				}

				if (!builtinFI)
					bytecode << op_push;
			}

			if (builtinFI)
			{
				bytecode << op_func << fidx;
			}
			else
			{
				bytecode << op_fcall << fi.nameHash;
				bytecode << op_trdx << (KIMLUINT)0;
			}
		}
		break;

	case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			this->Emit(exprcast->getValue());
			switch (exprcast->getCastType())
			{
			case KIML_BOOL:
				bytecode << op_cvb;
				break;
			case KIML_INT:
				bytecode << op_cvin;
				break;
			case KIML_REAL:
				bytecode << op_cvrn;
				break;
			case KIML_STRING:
				bytecode << op_cvs;
				break;
			case KIML_OBJECT:
				bytecode << op_cvo;
				break;
			}
		}
		break;
	}
}

bool Compiler::AnalyseSemanticsInclude(AstProgram *program)
{
	std::vector<AstStatement *> &statements = program->getStatements()->getStatements();
	bool res = true;

	for (auto it = statements.begin(); it != statements.end(); ++it)
	{
		AstStatement *stmt = *it;
		if (!stmt)
			continue;

		if (!this->PerformTypeCheckInclude(stmt))
			res = false;
	}

	return res;
}

bool Compiler::AnalyseSemantics(AstProgram *program)
{
	std::vector<AstStatement *> &statements = program->getStatements()->getStatements();
	KIMLCOMPILEERRORCODES err;
	bool res = true;
	
	// collect labels
	for (auto it = statements.begin(); it != statements.end(); ++it)
	{
		AstStatement *stmt = *it;
		if (!stmt || stmt->getType() != LABELDECL)
			continue;

		auto label = static_cast<AstLabel *>(stmt);
		err = this->labelTable->AddLabel(label->getName());
		if (err)
		{
			raiseCompileErrorNoReturn(err, (KIMLUINT)label->getLocation().first_line);
			res = false;
		}
	}

	for (auto it = statements.begin(); it != statements.end(); ++it)
	{
		AstStatement *stmt = *it;
		if (!stmt)
			continue;

		if (!this->PerformTypeCheck(stmt))
			res = false;
	}

	return res;
}

bool Compiler::PerformTypeCheckInclude(AstStatement *statement)
{
	KIMLUINT line = (KIMLUINT)statement->getLocation().first_line;
	KIMLCOMPILEERRORCODES err;

	switch (statement->getType())
	{
	case CONSTDEF:
		{
			auto constdef = static_cast<AstConstDef *>(statement);

			err = this->symbolTable->HasSymbol(constdef->getName());
			if (err == KCE_NONE)
				raiseCompileError(KCE_DUPLICATE_VAR, line);

			if (!this->PerformTypeCheck(constdef->getValue()))
				return false;

			if (!this->FoldExpression(constdef->getValue()))
				raiseCompileError(KCE_EXPRESSION_NONCONST, line);

			err = this->constTable->AddConst(constdef->getName(), *constdef->getValue()->getConstValue());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case ENUMDEF:
		{
			auto enumdef = static_cast<AstEnumDef *>(statement);

			const std::vector<KIMLCSTRING> &idents = enumdef->getIdents();
			KIMLINT idx = 0;
			for (auto it = idents.begin(); it != idents.end(); ++it)
			{
				err = this->symbolTable->HasSymbol(*it);
				if (err == KCE_NONE)
				{
					raiseCompileErrorNoReturn(KCE_DUPLICATE_VAR, line);
					continue;
				}

				Object obj(idx);
				err = this->constTable->AddConst(*it, obj);
				if (err)
				{
					raiseCompileErrorNoReturn(err, line);
					continue;
				}

				++idx;
			}
		}
		return true;

	case VARDECL:
		{
			auto vardecl = static_cast<AstVarDecl *>(statement);

			if (this->constTable->HasConst(vardecl->getVarName()))
				raiseCompileError(KCE_DUPLICATE_CONST, line);

			// add symbol
			err = this->symbolTable->AddSymbol(vardecl->getVarName(), vardecl->getVarType());
			if (err)
				raiseCompileError(err, line);

			if (vardecl->getDefaultValue())
				raiseCompileError(KCE_INCLUDE_VARDECL, line);
		}
		return true;

	case FUNCDECL:
		{
			auto funcdecl = static_cast<AstFuncDecl *>(statement);

			if (this->constTable->HasConst(funcdecl->getName()))
				raiseCompileError(KCE_DUPLICATE_CONST, line);

			auto paramtypes = funcdecl->getParams()->getTypes();
			FunctionInfo fi;
			fi.nameHash = kimlHashName(funcdecl->getName());
			fi.returnType = funcdecl->getReturnType();
			fi.setParams(funcdecl->getParams()->getTypes());

			err = this->funcTable->AddFunction(funcdecl->getName(), fi);
			if (err)
				raiseCompileError(err, line);

			if (funcdecl->getLabel())
				raiseCompileError(KCE_INCLUDE_FUNCDECL, line);
		}
		return true;

	case EMPTYLINE:
	case LABELDECL:
		return true;

	default:
		raiseCompileError(KCE_INCLUDE_ONLY, line);
	}
}

bool Compiler::PerformTypeCheck(AstStatement *statement)
{
	KIMLUINT line = (KIMLUINT)statement->getLocation().first_line;
	KIMLCOMPILEERRORCODES err;

	switch (statement->getType())
	{
	case EMPTYLINE:
		return true;

	case IOIN:
		{
			auto ioin = static_cast<AstIoIn *>(statement);
			if (ioin->getInputIdent())
			{
				 err = this->symbolTable->HasSymbol(ioin->getInputIdent());
				 if (err)
					 raiseCompileError(err, line);
			}
		}
		return true;

	case IOOUT:
		{
			auto ioout = static_cast<AstIoOut *>(statement);
			return this->PerformTypeCheck(ioout->getValue());
		}

	case VARDECL:
		{
			auto vardecl = static_cast<AstVarDecl *>(statement);

			if (this->constTable->HasConst(vardecl->getVarName()))
				raiseCompileError(KCE_DUPLICATE_CONST, line);

			// add symbol
			err = this->symbolTable->AddSymbol(vardecl->getVarName(), vardecl->getVarType());
			if (err)
				raiseCompileError(err, line);

			if (vardecl->getDefaultValue())
			{
				// check expression
				if (!this->PerformTypeCheck(vardecl->getDefaultValue()))
					return false;
				// check type match
				if (TypeMatchTable[vardecl->getVarType()][vardecl->getDefaultValue()->getDataType()] == K_TYPE_MISMATCHED)
					raiseCompileError(KCE_TYPE_MISMATCHED, line);
			}
		}
		return true;

	case VARSET:
		{
			auto varset = static_cast<AstVarSet *>(statement);
			// get symbol datatype
			KIMLTYPES vartype;
			err = this->symbolTable->GetSymbolType(varset->getVarName(), vartype);
			if (err)
				raiseCompileError(err, line);
			// check expression
			if (!this->PerformTypeCheck(varset->getValue()))
				return false;
			// check type match
			if (TypeMatchTable[vartype][varset->getValue()->getDataType()] == K_TYPE_MISMATCHED)
				raiseCompileError(KCE_TYPE_MISMATCHED, line);
		}
		return true;

	case VARDEL:
		{
			auto vardel = static_cast<AstVarDel *>(statement);
			err = this->symbolTable->RemoveSymbol(vardel->getVarName());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case STACKPUSH:
		{
			auto stackpush = static_cast<AstStackPush *>(statement);
			return this->PerformTypeCheck(stackpush->getValue());
		}

	case STACKPOP:
		{
			auto stackpop = static_cast<AstStackPop *>(statement);
			if (stackpop->getVarName())
			{
				err = this->symbolTable->HasSymbol(stackpop->getVarName());
				if (err)
					raiseCompileError(err, line);
			}
		}
		return true;

	case STACKPEEK:
		{
			auto stackpeek = static_cast<AstStackPeek *>(statement);
			err = this->symbolTable->HasSymbol(stackpeek->getVarName());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case STACKSWAP:
		return true;

	case STACKCLEAR:
		return true;

	case STACKREAD:
		{
			auto stackread = static_cast<AstStackRead *>(statement);
			if (stackread->getVarName())
			{
				err = this->symbolTable->HasSymbol(stackread->getVarName());
				if (err)
					raiseCompileError(err, line);
			}

			AstExpression *offsetExpr = stackread->getOffset();
			if (offsetExpr)
			{
				if (!this->PerformTypeCheck(offsetExpr))
					return false;

				if (!this->FoldExpression(offsetExpr))
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);

				if (offsetExpr->getConstValue()->getDataType() != KIML_INT)
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);
			}
		}
		return true;

	case STACKWRITE:
		{
			auto stackwrite = static_cast<AstStackWrite *>(statement);
			AstExpression *offsetExpr = stackwrite->getOffset();
			if (offsetExpr)
			{
				if (!this->PerformTypeCheck(offsetExpr))
					return false;

				if (!this->FoldExpression(offsetExpr))
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);

				if (offsetExpr->getConstValue()->getDataType() != KIML_INT)
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);
			}

			return this->PerformTypeCheck(stackwrite->getValue());
		}

	case TAPEREAD:
		{
			auto taperead = static_cast<AstTapeRead *>(statement);
			if (taperead->getDestVar())
			{
				err = this->symbolTable->HasSymbol(taperead->getDestVar());
				if (err)
					raiseCompileError(err, line);
			}

			AstExpression *idxexpr = taperead->getIndex();
			if (idxexpr)
			{
				if (!this->PerformTypeCheck(idxexpr))
					return false;

				if (!this->FoldExpression(idxexpr))
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);

				if (idxexpr->getConstValue()->getDataType() != KIML_INT)
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);
			}
		}
		return true;

	case TAPEWRITE:
		{
			auto tapewrite = static_cast<AstTapeWrite *>(statement);

			AstExpression *idxexpr = tapewrite->getIndex();
			if (idxexpr)
			{
				if (!this->PerformTypeCheck(idxexpr))
					return false;

				if (!this->FoldExpression(idxexpr))
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);

				if (idxexpr->getConstValue()->getDataType() != KIML_INT)
					raiseCompileError(KCE_EXPRESSION_NOTINT, line);
			}

			return this->PerformTypeCheck(tapewrite->getValue());
		}

	case TAPENEXT:
		return true;

	case TAPEPREV:
		return true;

	case TAPEMOVE:
		{
			auto tapemove = static_cast<AstTapeMove *>(statement);
			if (!this->PerformTypeCheck(tapemove->getValue()))
				return false;

			if (TypeMatchTable[KIML_INT][tapemove->getValue()->getDataType()] == K_TYPE_MISMATCHED)
				raiseCompileError(KCE_TYPE_MISMATCHED, line);
		}
		return true;

	case CTRLGOTO:
		{
			auto ctrlgoto = static_cast<AstCtrlGoto *>(statement);
			err = this->labelTable->HasLabel(ctrlgoto->getLabel());
			if (err)
				raiseCompileError(err, line);

			if (ctrlgoto->getCondition())
				return this->PerformTypeCheck(ctrlgoto->getCondition());
			else
				return true;
		}

	case CTRLCALL:
		{
			auto ctrlcall = static_cast<AstCtrlCall *>(statement);
			err = this->labelTable->HasLabel(ctrlcall->getLabel());
			if (err)
				raiseCompileError(err, line);

			if (ctrlcall->getCondition())
				return this->PerformTypeCheck(ctrlcall->getCondition());
			else
				return true;
		}

	case CTRLRETURN:
		return true;

	case CTRLEND:
		return true;

	case FUNCDECL:
		{
			auto funcdecl = static_cast<AstFuncDecl *>(statement);

			if (this->constTable->HasConst(funcdecl->getName()))
				raiseCompileError(KCE_DUPLICATE_CONST, line);

			auto paramtypes = funcdecl->getParams()->getTypes();
			FunctionInfo fi;
			fi.nameHash = kimlHashName(funcdecl->getName());
			fi.returnType = funcdecl->getReturnType();
			fi.setParams(funcdecl->getParams()->getTypes());

			err = this->funcTable->AddFunction(funcdecl->getName(), fi);
			if (err)
				raiseCompileError(err, line);

			if (funcdecl->getLabel())
			{
				err = this->labelTable->HasLabel(funcdecl->getLabel());
				if (err)
					raiseCompileError(err, line);
			}
		}
		return true;

	case FUNCCALL:
		{
			auto funccall = static_cast<AstFuncCall *>(statement);
			FunctionInfo fi;
			err = this->funcTable->GetFunction(funccall->getName(), fi);
			if (err)
				raiseCompileError(err, line);

			auto args = funccall->getArgs()->getExpressions();
			if (args.size() != fi.paramCount)
				raiseCompileError(KCE_INCONSISTENT_ARG_COUNT, line);

			bool result = true;
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				if (!this->PerformTypeCheck(arg))
				{
					result = false;
					continue;
				}

				if (TypeMatchTable[fi.paramTypes[idx]][arg->getDataType()] == K_TYPE_MISMATCHED)
				{
					result = false;
					raiseCompileErrorNoReturn(KCE_TYPE_MISMATCHED, line);
				}
			}

			return result;
		}

	case EVENTHANDLE:
		{
			auto eventhandle = static_cast<AstEventHandle *>(statement);
			err = this->labelTable->HasLabel(eventhandle->getLabel());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case EVENTUNHANDLE:
		{
			auto eventunhandle = static_cast<AstEventUnhandle *>(statement);
			err = this->labelTable->HasLabel(eventunhandle->getLabel());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case EVENTRAISE:
		return true;

	case CONSTDEF:
		{
			auto constdef = static_cast<AstConstDef *>(statement);

			err = this->symbolTable->HasSymbol(constdef->getName());
			if (err == KCE_NONE)
				raiseCompileError(KCE_DUPLICATE_VAR, line);

			if (!this->PerformTypeCheck(constdef->getValue()))
				return false;

			if (!this->FoldExpression(constdef->getValue()))
				raiseCompileError(KCE_EXPRESSION_NONCONST, line);

			err = this->constTable->AddConst(constdef->getName(), *constdef->getValue()->getConstValue());
			if (err)
				raiseCompileError(err, line);
		}
		return true;

	case ENUMDEF:
		{
			auto enumdef = static_cast<AstEnumDef *>(statement);

			const std::vector<KIMLCSTRING> &idents = enumdef->getIdents();
			KIMLINT idx = 0;
			for (auto it = idents.begin(); it != idents.end(); ++it)
			{
				err = this->symbolTable->HasSymbol(*it);
				if (err == KCE_NONE)
				{
					raiseCompileErrorNoReturn(KCE_DUPLICATE_VAR, line);
					continue;
				}

				Object obj(idx);
				err = this->constTable->AddConst(*it, obj);
				if (err)
				{
					raiseCompileErrorNoReturn(err, line);
					continue;
				}

				++idx;
			}
		}
		return true;

	default:
		return true;
	}
}

bool Compiler::PerformTypeCheck(AstExpression *expression)
{
	KIMLUINT line = (KIMLUINT)expression->getLocation().first_line;
	KIMLCOMPILEERRORCODES err;

	switch (expression->getType())
	{
	case EXPR_NONE:
		return true;

	case EXPR_IDENT:
		{
			auto exprident = static_cast<AstIdent *>(expression);
			KIMLTYPES vartype;

			Object obj;
			if (this->constTable->GetConst(exprident->getIdent(), obj))
			{
				vartype = obj.getDataType();
			}
			else
			{
				err = this->symbolTable->GetSymbolType(exprident->getIdent(), vartype);
				if (err)
					raiseCompileError(err, line);
			}
			
			expression->setDataType(vartype);
		}
		return true;

	case EXPR_TRUE:
	case EXPR_FALSE:
	case EXPR_NULL:
	case EXPR_INT:
	case EXPR_REAL:
	case EXPR_STRING:
		return true;

	case EXPR_BIN:
		{
			auto exprbin = static_cast<AstBinExpr *>(expression);
			if (!this->PerformTypeCheck(exprbin->getLeft()))
				return false;
			if (!this->PerformTypeCheck(exprbin->getRight()))
				return false;

			switch (exprbin->getOperator())
			{
			case BINOP_EQU:
			case BINOP_NEQ:
				expression->setDataType(KIML_BOOL);
				return true;

			case BINOP_LES:
			case BINOP_LEQ:
			case BINOP_GRT:
			case BINOP_GEQ:
				{
					KIMLTYPES t = HigherType(exprbin->getLeft()->getDataType(), exprbin->getRight()->getDataType());
					if (!IsValidArithmetic(t))
						raiseCompileError(KCE_TYPE_MISMATCHED, line);

					expression->setDataType(KIML_BOOL);
				}
				return true;

			case BINOP_ADD:
			case BINOP_SUB:
			case BINOP_MUL:
			case BINOP_DIV:
			case BINOP_MOD:
			case BINOP_POW:
				{
					KIMLTYPES t = HigherType(exprbin->getLeft()->getDataType(), exprbin->getRight()->getDataType());
					if (!IsValidArithmetic(t))
						raiseCompileError(KCE_TYPE_MISMATCHED, line);

					expression->setDataType(t);
				}
				return true;

			case BINOP_CAT:
				expression->setDataType(KIML_STRING);
				return true;

			case BINOP_AND:
			case BINOP_OR:
			case BINOP_XOR:
				expression->setDataType(KIML_BOOL);
				return true;
			}
		}
		return true;

	case EXPR_UN:
		{
			auto exprun = static_cast<AstUnExpr *>(expression);
			if (!this->PerformTypeCheck(exprun->getExpression()))
				return false;

			switch (exprun->getOperator())
			{
			case UNOP_NEG:
				if (!IsValidArithmetic(exprun->getExpression()->getDataType()))
					raiseCompileError(KCE_TYPE_MISMATCHED, line);

				expression->setDataType(exprun->getExpression()->getDataType());
				return true;
				
			case UNOP_NOT:
				expression->setDataType(KIML_BOOL);
				return true;
			}
		}
		return true;

	case EXPR_FUNC:
		{
			auto exprfunc = static_cast<AstExprFunc *>(expression);
			auto args = exprfunc->getArgs()->getExpressions();
			FunctionInfo fi;

			KIMLUINT fidx;
			const FunctionInfo *builtinFI = FunctionTable::GetBuiltinFunctionInfo(exprfunc->getName(), fidx);
			if (builtinFI)
			{
				fi = *builtinFI;
			}
			else
			{
				err = this->funcTable->GetFunction(exprfunc->getName(), fi);
				if (err)
					raiseCompileError(err, line);
			}

			if (fi.paramCount != args.size())
				raiseCompileError(KCE_INCONSISTENT_ARG_COUNT, line);

			bool result = true;
			int idx = 0;
			for (auto it = args.begin(); it != args.end(); ++it, ++idx)
			{
				AstExpression *arg = *it;
				if (!this->PerformTypeCheck(arg))
				{
					result = false;
					continue;
				}

				if (TypeMatchTable[fi.paramTypes[idx]][arg->getDataType()] == K_TYPE_MISMATCHED)
				{
					result = false;
					raiseCompileErrorNoReturn(KCE_TYPE_MISMATCHED, line);
				}
			}

			expression->setDataType(fi.returnType);
			return result;
		}

	case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			if (!this->PerformTypeCheck(exprcast->getValue()))
				return false;
		}
		return true;

	default:
		return true;
	}
}

bool Compiler::FoldExpression(AstExpression *expression)
{
	if (expression->getConstValue())
		return true;

	switch (expression->getType())
	{
	case EXPR_TRUE:
		expression->setConstValue(new Object(true));
		return true;

	case EXPR_FALSE:
		expression->setConstValue(new Object(false));
		return true;

	case EXPR_NULL:
		expression->setConstValue(new Object((KIMLOBJECT)nullptr));
		return true;

	case EXPR_INT:
		expression->setConstValue(new Object(static_cast<AstConstInt *>(expression)->getValue()));
		return true;

	case EXPR_REAL:
		expression->setConstValue(new Object(static_cast<AstConstReal *>(expression)->getValue()));
		return true;

	case EXPR_STRING:
		expression->setConstValue(new Object(static_cast<AstConstString *>(expression)->getValue()));
		return true;

	case EXPR_IDENT:
		{
			auto exprident = static_cast<AstIdent *>(expression);
			Object obj;
			if (this->constTable->GetConst(exprident->getIdent(), obj))
			{
				expression->setConstValue(new Object(obj));
				return true;
			}
			else
			{
				return false;
			}
		}

	case EXPR_BIN:
		{
			auto binexpr = static_cast<AstBinExpr *>(expression);
			auto left = binexpr->getLeft();
			auto right = binexpr->getRight();

			if (!this->FoldExpression(left) || !this->FoldExpression(right))
				return false;

			switch (binexpr->getOperator())
			{
			case BINOP_EQU:
				expression->setConstValue(new Object(*left->getConstValue() == *right->getConstValue()));
				return true;
			case BINOP_NEQ:
				expression->setConstValue(new Object(*left->getConstValue() != *right->getConstValue()));
				return true;
			case BINOP_LES:
				expression->setConstValue(new Object(*left->getConstValue() < *right->getConstValue()));
				return true;
			case BINOP_LEQ:
				expression->setConstValue(new Object(*left->getConstValue() <= *right->getConstValue()));
				return true;
			case BINOP_GRT:
				expression->setConstValue(new Object(*left->getConstValue() > *right->getConstValue()));
				return true;
			case BINOP_GEQ:
				expression->setConstValue(new Object(*left->getConstValue() >= *right->getConstValue()));
				return true;
			case BINOP_ADD:
				expression->setConstValue(new Object(*left->getConstValue() + *right->getConstValue()));
				return true;
			case BINOP_SUB:
				expression->setConstValue(new Object(*left->getConstValue() - *right->getConstValue()));
				return true;
			case BINOP_MUL:
				expression->setConstValue(new Object(*left->getConstValue() * *right->getConstValue()));
				return true;
			case BINOP_DIV:
				expression->setConstValue(new Object(*left->getConstValue() / *right->getConstValue()));
				return true;
			case BINOP_MOD:
				expression->setConstValue(new Object(*left->getConstValue() % *right->getConstValue()));
				return true;
			case BINOP_POW:
				expression->setConstValue(new Object(Object::Pow(*left->getConstValue(), *right->getConstValue())));
				return true;
			case BINOP_CAT:
				expression->setConstValue(new Object(Object::Concat(*left->getConstValue(), *right->getConstValue())));
				return true;
			case BINOP_AND:
				expression->setConstValue(new Object(*left->getConstValue() && *right->getConstValue()));
				return true;
			case BINOP_OR:
				expression->setConstValue(new Object(*left->getConstValue() || *right->getConstValue()));
				return true;
			case BINOP_XOR:
				expression->setConstValue(new Object(*left->getConstValue() ^ *right->getConstValue()));
				return true;
			default:
				return false;
			}
		}

	case EXPR_UN:
		{
			auto unexpr = static_cast<AstUnExpr *>(expression);
			auto expr = unexpr->getExpression();

			if (!this->FoldExpression(expr))
				return false;

			switch (unexpr->getOperator())
			{
			case UNOP_NOT:
				expression->setConstValue(new Object(!*expr->getConstValue()));
				return true;
			case UNOP_NEG:
				expression->setConstValue(new Object(-*expr->getConstValue()));
				return true;
			default:
				return false;
			}
		}

	case EXPR_CAST:
		{
			auto exprcast = static_cast<AstCast *>(expression);
			auto expr = exprcast->getValue();

			if (!this->FoldExpression(expr))
				return false;

			switch (exprcast->getDataType())
			{
			case KIML_BOOL:
				expression->setConstValue(new Object(expr->getConstValue()->getBool() == TRUE));
				return true;
			case KIML_INT:
				expression->setConstValue(new Object(expr->getConstValue()->getInt()));
				return true;
			case KIML_REAL:
				expression->setConstValue(new Object(expr->getConstValue()->getFloat()));
				return true;
			case KIML_STRING:
				{
					KIMLCSTRING s = expr->getConstValue()->getString();
					expression->setConstValue(new Object(s));
					delete []s;
					return true;
				}
			}
		}

	default:
		return false;
	}
}
