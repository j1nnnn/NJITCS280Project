
#include "parseRun.h"
#include "val.h"
#include "lex.h"
#include <iostream>
#include <fstream>


using namespace std;

int main(int argc, char* argv[]) {
    string args[argc];
    ifstream file;
    istream *in;
    int linenum = 1;

    if( argc == 1 ) {
       in = &cin;
    }
    else if( argc == 2 ) {
        file.open(argv[1]);
        if(file.is_open() == false) {
            cout << "COULD NOT OPEN FILE " << argv[1] << endl;
            return 0;
        }
        in = &file;
    }
    else {
        cout << "TOO MANY FILENAMES" << endl;
        return 0;
    }

    if ( Prog(file, linenum )) {
        cout << endl;
        cout << "Successful Execution!" << endl;
        return 0;
    }
    else {
        cout << endl;
        cout << "Unsuccessful Interpretation..." << endl;
        cout << "Number of Syntax Errors: " << error_count << endl;
        return 0;
    }

}

//Program is: Prog := begin StmtList end
bool Prog(istream& in, int& line)
{
	bool sl = false;
	LexItem tok = Parser::GetNextToken(in, line);
	//cout << "in Prog" << endl;
	
	if (tok.GetToken() == BEGIN) {
		sl = StmtList(in, line);
		if( !sl  )
			ParseError(line, "No statements in program");
		if( error_count > 0 )
			return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	
	if (tok.GetToken() == END)
		return true;
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else
		return false;
}

// StmtList is a Stmt followed by semicolon followed by a StmtList
 bool StmtList(istream& in, int& line) {
 	//cout << "in StmtList" << endl;
	bool status = Stmt(in, line);
	
	if( !status )
		return false;
	LexItem tok = Parser::GetNextToken(in, line);
	
	if( tok == SCOMA ) {
		status = StmtList(in, line);
	}
	else if (tok == ERR) {
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else if (tok == END) {
		Parser::PushBackToken(tok);
		return true;
	}
	else {
		ParseError(line, "Missing semicolon");
		return false;
	}
	return status;
}

//Stmt is either a PrintStmt, IfStmt, or an AssigStmt
bool Stmt(istream& in, int& line) {
	bool status;
	//cout << "in Stmt" << endl;
	LexItem t = Parser::GetNextToken(in, line);
	
	switch( t.GetToken() ) {

	case PRINT:

	    status = PrintStmt(in, line);
		//cout << "status: " << (status? true:false) <<endl;
		break;

	case IF:
		status = IfStmt(in, line);
		break;

	case IDENT:
        Parser::PushBackToken(t);
		status = AssignStmt(in, line);
		break;

	case END:
		Parser::PushBackToken(t);
		return true;
	case ERR:
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << t.GetLexeme() << ")" << endl;
		return false;
	case DONE:
		return false;

	default:
		ParseError(line, "Invalid statement");
		return false;
	}

	return status;
}

//PrintStmt:= print ExprList
bool PrintStmt(istream& in, int& line) {
	//cout << "in PrintStmt" << endl;
	//create an empty queue of Value objects.

	ValQue = new queue<Value>;

	bool ex = ExprList(in, line);
	
	if( !ex ) {
		ParseError(line, "Missing expression after print");
		//Empty the queue and delete.
		while(!(*ValQue).empty()) {
		    ValQue->pop();
		}
		delete ValQue;
		return false;
	}
	//Evaluate: print out the list of expressions values
    LexItem t = Parser::GetNextToken(in, line);
	if (t.GetToken() == SCOMA) {
	    //Execute the statement after making sure
	    //the semicolon is seen.
	    while ( !(*ValQue).empty() ) {
	        Value nextVal = (*ValQue).front();
	        cout << nextVal;
	        ValQue->pop();
	    }
	    cout << endl;
	}
	Parser::PushBackToken(t);
	return ex;
}

//IfStmt:= if (Expr) then Stmt
bool IfStmt(istream& in, int& line) {
    //cout << "In IfStmt" << endl;

    bool ex = false;
    //bool st = false;
	Value newVal;
	LexItem t;

	if ((t=Parser::GetNextToken(in,line)) != LPAREN ) {

		ParseError(line, "Missing Left Parenthesis");
		return false;
	}

	ex = Expr(in, line, newVal);
    //Expr must be of an integer type
    //If the expression value is nonzero, then the Stmt is executed, otherwise it is note
	if( !ex ) {
		ParseError(line, "Missing if statement expression");
		return false;
	}

	if((t=Parser::GetNextToken(in, line)) != RPAREN ) {
		ParseError(line, "Missing Right Parenthesis");
		return false;
	}


	if((t=Parser::GetNextToken(in, line)) != THEN ) {

		ParseError(line, "Missing THEN");
		return false;
	}

    if(!newVal.IsInt()) {
        ParseError(line, "Run-Time Error-Illegal Type for If statement Expression");
        return false;
    }
    if(newVal.GetInt() == 0) {
        while((t = Parser::GetNextToken(in, line)) != SCOMA) {

        }
        Parser::PushBackToken(t);
        return true;
    }


	bool st = Stmt(in, line);

	if( !st ) {
		ParseError(line, "Missing statement for if");
		return false;
	}

	//Evaluate: execute the if statement
	//Check type of the expr

	return st;
}

//Var:= ident
bool Var(istream& in, int& line, LexItem & tok)
{
	//called only from the AssignStmt function
	string identstr;
	//cout << "in Var" << endl;
	tok = Parser::GetNextToken(in, line);
	//if identstr isn't found in symbol table then you add with its associated value
	//otherwise (else) that means it already exists and is being reassigned so you have to make sure its not breaking any of the assignment rules
	//i.e. an existing variable that holds an int can't be reassigned to a string and vice versa
	if (tok == IDENT){
		identstr = tok.GetLexeme();
		if (defVar.find(identstr) == defVar.end()) {
            //cout << "Setting variable (" << identstr << ") to true in defVar map" << endl;
		    defVar[identstr] = true;
		    Value val;
		    symbolTable[identstr] = val;
        }
		return true;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	return false;
}

//AssignStmt:= Var = Expr
bool AssignStmt(istream& in, int& line) {
	//cout << "in AssignStmt" << endl;
	bool varstatus = false, status = false;
	LexItem t;
	Value newVal;
    //call Value object

	varstatus = Var( in, line, t);
	//cout << "varstatus:" << varstatus << endl;
	
	if (varstatus){
        string varname = t.GetLexeme();
	    if ((t=Parser::GetNextToken(in, line)) == EQ){
	        status = Expr(in, line, newVal);
            //cout << "NewVal " << newVal << endl;
            //cout << "SymbolTable " << symbolTable[varname] << endl;
            //cout << "SymbolGettype " << symbolTable[varname].GetType() << endl;
            //cout << "NewValType" << newVal.GetType() << endl;
	        if(!status) {
				ParseError(line, "Missing Expression in Assignment Statment");
				return status;
			}
            else if (newVal.GetType() == 1 && symbolTable[varname].GetType() == 0) {
                symbolTable[varname] = Value((int)newVal.GetReal());
            }
            else if (newVal.GetType() != 2 && symbolTable[varname].GetType() == 2) {
                ParseError(line, "Illegal Assignment Operation");
                return false;
            }
            else if(newVal.GetType() == symbolTable[varname].GetType()) {
                symbolTable[varname] = newVal;
            }

            else if(newVal.IsStr() == symbolTable[varname].IsStr()) {
                symbolTable[varname] = newVal;
            }

            else if(newVal.IsInt() == symbolTable[varname].IsInt() || newVal.IsReal() == symbolTable[varname].IsReal()) {
                //newVal.GetInt();
                symbolTable[varname] = newVal;
            }

            else {
                //symbolTable[varname] = newVal;
                //cout << "Test : " << newVal << endl;
                ParseError(line, "Illegal Assignment Operation");
                return false;
            }

		}
		else if(t.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << t.GetLexeme() << ")" << endl;
			return false;
		}
		else {
			ParseError(line, "Missing Assignment Operator =");
			return false;
		}

	}
	else {
		ParseError(line, "Missing Left-Hand Side Variable in Assignment statement");
		return false;
	}
	return status;	
}

//ExprList:= Expr {,Expr}
bool ExprList(istream& in, int& line) {
	bool status = false;

    Value newVal;
	//cout << "in ExprList" << endl;


	status = Expr(in, line, newVal);
    ValQue->push(newVal);
	if(!status){
		ParseError(line, "Missing Expression");
		return false;
	}

	LexItem tok = Parser::GetNextToken(in, line);
	
	if (tok == COMA) {
		status = ExprList(in, line);


	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	else{
		Parser::PushBackToken(tok);
		return true;
	}

	return status;
}

//Expr:= Term {(+|-) Term}
bool Expr(istream& in, int& line, Value& retVal) {
    //Value newVal;
    //cout << "in Expr" << endl;
    bool t1 = Term(in, line, retVal);
	LexItem tok;
	if( !t1 ) {
		return false;
	}
	
	tok = Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}
	while ( tok == PLUS || tok == MINUS ) 
	{

	    Value newVal;
	    t1 = Term(in, line, newVal);
		if( !t1 ) 
		{
			ParseError(line, "Missing expression after operator");
			return false;
		}
		if(retVal.IsStr() || newVal.IsStr()) {
		    if(retVal.GetType() != newVal.GetType()) {
		        ParseError(line, "Run time Error-Illegal Mixed Type operation");
		        return false;
		    }
		    ParseError(line, "Run-time Error-Illegal String Type operation");
		    return false;
		}
		/*if((retVal.IsInt() || newVal.IsInt()) && (retVal.IsReal() || newVal.IsReal())) {
		    if(retVal.IsInt()) {
		        int a = retVal.GetInt();
		        float b = newVal.GetReal();
		        if(tok == PLUS) {
                    retVal = Value((int)(a+b));
		        }
		        else if (tok == MINUS){
		            retVal = Value((int)(a-b));
		        }
		        //retVal = Value((int)(a+b));
		    }
		    else {
		        float a = retVal.GetReal();
		        int b = newVal.GetInt();
                if(tok == PLUS) {
                    retVal = Value((int)(a+b));
                }
                else if (tok == MINUS){
                    retVal = Value((int)(a-b));
                }
		    }
		}
		else {
            if( tok == PLUS) {
                retVal = retVal + newVal;
            }
            if (tok == MINUS) {
                retVal = retVal - newVal;
            }
		}*/
        if( tok == PLUS) {
            retVal = retVal + newVal;
        }
        if (tok == MINUS) {
            retVal = retVal - newVal;
        }

		tok = Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}		
		
		//Evaluate: evaluate the expression for addition or subtraction
	}
	//retVal = newVal;
	Parser::PushBackToken(tok);
	return true;
}

//Term:= Factor {(*|/) Factor}
bool Term(istream& in, int& line, Value& retVal) {
	//cout << "in Term" << endl;
	//Value newVal;
	bool t1 = Factor(in, line, retVal);
	LexItem tok;
	//cout << "status of factor1: " << t1<< endl;
	if( !t1 ) {
		return false;
	}
	
	tok	= Parser::GetNextToken(in, line);
	if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
	}
	while ( tok == MULT || tok == DIV  )
	{
		Value newVal;
	    t1 = Factor(in, line, newVal);
		//cout << "status of factor2: " << t1<< endl;
		if( !t1 ) {
			ParseError(line, "Missing expression after operator");
			return false;
		}
        if(retVal.IsStr() || newVal.IsStr()) {
            if(retVal.GetType() != newVal.GetType()) {
                ParseError(line, "Run time Error: different Types");
                return false;
            }
            ParseError(line, "Run time Error: two strings");
            return false;
        }
        /*if((retVal.IsInt() || newVal.IsInt()) && (retVal.IsReal() || newVal.IsReal())) {
            if(retVal.IsInt()) {
                int a = retVal.GetInt();
                float b = newVal.GetReal();
                if(tok == MULT) {
                    retVal = Value((int)(a*b));
                }
                else if (tok == DIV){
                    retVal = Value((int)(a/b));
                }
                //retVal = Value((int)(a+b));
            }
            else {
                float a = retVal.GetReal();
                int b = newVal.GetInt();
                if(tok == MULT) {
                    retVal = Value((int)(a*b));
                }
                else if (tok == DIV){
                    retVal = Value((int)(a/b));
                }
            }
        }
        else {
            if( tok == MULT) {
                retVal = retVal * newVal;
            }
            if (tok == DIV) {
                retVal = retVal / newVal;
            }
        }*/
        if( tok == MULT) {
            retVal = retVal * newVal;
        }
        if (tok == DIV) {
            retVal = retVal / newVal;
        }

		tok	= Parser::GetNextToken(in, line);
		if(tok.GetToken() == ERR){
			ParseError(line, "Unrecognized Input Pattern");
			cout << "(" << tok.GetLexeme() << ")" << endl;
			return false;
		}
		//Evaluate: evaluate the expression for multiplication or division
	}
	//retVal = newVal;
	Parser::PushBackToken(tok);
	return true;
}

//Factor := ident | iconst | rconst | sconst | (Expr)
bool Factor(istream& in, int& line, Value &retVal) {
	//cout << "in Factor" << endl;
	Value newVal;
	LexItem tok = Parser::GetNextToken(in, line);
	if( tok == IDENT ) {
		string identstr = tok.GetLexeme();

        //check if the var is defined
		if (!(defVar.find(identstr)->second))
		{
			ParseError(line, "Undefined Variable");
			return false;
		}
		else {
		    retVal = symbolTable.find(identstr)->second;
            //ValQue->push(retVal);
		    return true;
		}

	}
	else if( tok == ICONST ) {
		//convert the string of digits to an integer number
		//create a Val object for ICONST and enter into sysmol table

		int val;
		//string lexeme = tok.GetLexeme();
		val = stoi(tok.GetLexeme());
		//cout << "integer const: " << val << endl;
		Value newVal(val);
		retVal = newVal;
        //ValQue->push(retVal);

        /*for (auto const& x : symbolTable) {


            cout << x.first //key
                 << ":"
                 << x.second //value
                 << endl;

        }*/

		return true;
	}
	else if( tok == SCONST ) {
		//
		//create a Val object for SCONST and enter into symbol table

		string sval = tok.GetLexeme();
		//Value value;
		Value newVal(sval);
		retVal = newVal;
        //ValQue->push(retVal);
		//cout << "string const: " << sval << endl;
        /*for (auto const& x : symbolTable) {


            cout << x.first //key
                 << ":"
                 << x.second //value
                 << endl;

        }*/
        return true;
	}
	else if( tok == RCONST ) {
		//convert the string of digits to real number
		//create a Val object for RCONST and enter into symbol table
		float rval;
		Value value;
		//string lexeme = tok.GetLexeme();
		rval = stof(tok.GetLexeme());
		//cout << "real const: " << rval << endl;
		Value newVal(rval);
		retVal = newVal;
        //ValQue->push(retVal);

		/*for (auto const& x : symbolTable) {

            cout << x.first //key
                 << ":"
                 << x.second //value
                 << endl;

        }*/

        return true;
	}
	else if( tok == LPAREN ) {
		bool ex = Expr(in, line, newVal);
		if( !ex ) {
			ParseError(line, "Missing expression after (");
			return false;
		}
		if( Parser::GetNextToken(in, line) == RPAREN )
			return ex;

		ParseError(line, "Missing ) after expression");
		return false;
	}
	else if(tok.GetToken() == ERR){
		ParseError(line, "Unrecognized Input Pattern");
		cout << "(" << tok.GetLexeme() << ")" << endl;
		return false;
	}

	ParseError(line, "Unrecognized input");
	return 0;
}



