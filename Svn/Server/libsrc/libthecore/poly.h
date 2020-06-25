#ifndef __POLY_POLY_H__
#define __POLY_POLY_H__

#include "symtable.h"

#include <string>
#include <vector>
#include <list>

class CPoly
{
	public:
		CPoly();
		virtual ~CPoly();

		int32_t	Analyze(const char * pszStr = nullptr);
		double	Eval();
		void	SetStr(const std::string & str);
		int32_t	SetVar(const std::string & strName, double dVar);
		double GetVar(const std::string & strName);
		void	Clear();

	protected:
		int32_t		my_irandom(double start, double end);
		double		my_frandom(double start, double end);

		void		init();
		int32_t		insert(const std::string & s, int32_t tok);
		int32_t		find(const std::string & s);
		void		emit(int32_t t,int32_t tval);
		void		match(int32_t t); 
		void		expo(); 
		void		factor(); 
		void		term(); 
		int32_t		lexan();
		void		error();
		void		expr();

		int32_t		iToken;
		double		iNumToken;
		int32_t		iLookAhead;
		int32_t		iErrorPos;
		bool		ErrorOccur;
		uint32_t	uiLookPos;

		std::vector<int32_t>		tokenBase;
		std::vector<double>		numBase;
		std::vector<CSymTable *>	lSymbol;
		std::vector<int32_t>		SymbolIndex;
		int32_t				STSize;
		std::string			strData;

};

#endif 
