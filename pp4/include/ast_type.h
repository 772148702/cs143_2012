/* File: ast_type.h
 * ----------------
 * In our parse tree, Type nodes are used to represent and
 * store type information. The base Type class is used
 * for built-in types, the NamedType for classes and interfaces,
 * and the ArrayType for arrays of other types.  
 *
 * pp3: You will need to extend the Type classes to implement
 * the type system and rules for type equivalency and compatibility.
 */
 
#ifndef _H_ast_type
#define _H_ast_type

#include "ast.h"
#include "list.h"
#include "errors.h"
#include "codegen.h"
#include <iostream>
using namespace std;


class Type : public Node 
{
  protected:
    char *typeName;

  public :
    static Type *intType, *doubleType, *boolType, *voidType,
                *nullType, *stringType, *errorType;

    Type(yyltype loc) : Node(loc) {}
    Type() : Node() {}
    Type(const char *str);
    
    virtual void PrintToStream(ostream& out) { out << typeName; }
    friend ostream& operator<<(ostream& out, Type *t) { t->PrintToStream(out); return out; }
    virtual bool IsEquivalentTo(Type *other);
    virtual bool IsEqualTo(Type *other) {return this==other;}
    virtual void ReportNotDeclaredIdentifier(reasonT reason){return;}
    virtual const char* GetName() {return typeName;}
    virtual const char* Name() {return typeName;}
    virtual bool IsPrimitive() {return true;}
    virtual BuiltIn GetPrint();
    virtual int GetMemBytes() { return CodeGenerator::VarSize; }
};
//user define type
class NamedType : public Type 
{
  protected:
    Identifier *id;
    
  public:
    NamedType(Identifier *i);
    
    void PrintToStream(ostream& out) override { out << id; }
    void ReportNotDeclaredIdentifier(reasonT reason) override ;
    bool IsEqualTo(Type *other) override ;
    bool IsEquivalentTo(Type *other) override ;

    const char* GetName() {return id->GetName();}
    const char* Name() override {return id->Name();}
    bool IsPrimitive() override  {return false;}
    Identifier* GetId() {return id;}
    BuiltIn GetPrint() override;
};

class ArrayType : public Type 
{
  protected:
    Type *elemType;

  public:
    ArrayType(yyltype loc, Type *elemType);
    ArrayType(Type *elemType);

    void PrintToStream(ostream& out) override { out << elemType << "[]"; }
    void ReportNotDeclaredIdentifier(reasonT reason) override ;
    bool IsEqualTo(Type *other) override ;
    bool IsEquivalentTo(Type *other) override ;

    const char* Name() override {return elemType->Name();}
    bool IsPrimitive() override {return false;}

    Type* GetElemType() {return elemType;}
    BuiltIn GetPrint() override ;
};

 
#endif
