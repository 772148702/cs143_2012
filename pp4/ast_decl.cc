/* File: ast_decl.cc
 * -----------------
 * Implementation of Decl node classes.
 */
#include "include/ast_decl.h"
#include "include/ast_type.h"
#include "include/ast_stmt.h"
        
         
Decl::Decl(Identifier *n) : Node(*n->GetLocation()) {
    Assert(n != NULL);
    (id=n)->SetParent(this);
    scope = new Scope;
}

bool Decl::IsEquivalentTo(Decl *other) {
    return true;
}

void Decl::BuildScope() {

}

VarDecl::VarDecl(Identifier *n, Type *t) : Decl(n) {
    Assert(n != NULL && t != NULL);
    (type=t)->SetParent(this);
}

bool VarDecl::IsEquivalentTo(Decl *other) {
    VarDecl *varDecl = dynamic_cast<VarDecl*>(other);
    if(varDecl==NULL)
        return false;
    return type->IsEquivalentTo(varDecl->type);
}

void VarDecl::Check() {
    CheckType();
}

void VarDecl::CheckType() {
    if(type->IsPrimitive()) return;

    Scope *s = scope;
    Node* temp = this;
    while((temp)!=NULL) {
        s = temp->GetScope();
        Decl *d;
        if((d=s->table->Lookup(type->Name()))!=NULL) {
            if(dynamic_cast<ClassDecl*>(d)==NULL&&
                    dynamic_cast<InterfaceDecl*>(d)==NULL)
                type->ReportNotDeclaredIdentifier(LookingForType);

            return;
        }
        temp = temp->GetParent();
    }
    type->ReportNotDeclaredIdentifier(LookingForType);
}


int VarDecl::GetMemBytes() {
    return CodeGenerator::VarSize;
}




ClassDecl::ClassDecl(Identifier *n, NamedType *ex, List<NamedType*> *imp, List<Decl*> *m) : Decl(n) {
    // extends can be NULL, impl & mem may be empty lists but cannot be NULL
    Assert(n != NULL && imp != NULL && m != NULL);     
    extends = ex;
    if (extends) extends->SetParent(this);
    (implements=imp)->SetParentAll(this);
    (members=m)->SetParentAll(this);
}


void ClassDecl::BuildScope() {

    scope->SetClassDecl(this);

    scope->AddDecl(this);


    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->AddDecl(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->BuildScope();
}

void ClassDecl::CheckExtends() {
    if (extends == NULL)
        return;
    Node* tempNode = this->GetParent();
    Scope* s = tempNode->GetScope();
    Decl *lookup = s->table->Lookup(extends->Name());
    if (dynamic_cast<ClassDecl*>(lookup) == NULL)
        extends->ReportNotDeclaredIdentifier(LookingForClass);
}

void ClassDecl::CheckImplements() {
    Scope *s = scope->GetParent();

    for (int i = 0, n = implements->NumElements(); i < n; ++i) {
        NamedType *nth = implements->Nth(i);
        Decl *lookup = s->table->Lookup(implements->Nth(i)->Name());

        if (dynamic_cast<InterfaceDecl*>(lookup) == NULL)
            nth->ReportNotDeclaredIdentifier(LookingForInterface);
    }
}

void ClassDecl::CheckExtendedMembers(NamedType *extType) {
    if (extType == NULL)
        return;

    Decl *lookup = scope->GetParent()->table->Lookup(extType->Name());
    ClassDecl *extDecl = dynamic_cast<ClassDecl*>(lookup);
    if (extDecl == NULL)
        return;

    CheckExtendedMembers(extDecl->extends);
    CheckAgainstScope(extDecl->scope);
}

void ClassDecl::CheckImplementedMembers(NamedType *impType) {
    Decl *lookup = scope->GetParent()->table->Lookup(impType->Name());
    InterfaceDecl *intDecl = dynamic_cast<InterfaceDecl*>(lookup);
    if (intDecl == NULL)
        return;

    CheckAgainstScope(intDecl->GetScope());
}

void ClassDecl::CheckImplementsInterfaces() {
    Scope *s = scope->GetParent();

    for (int i = 0, n = implements->NumElements(); i < n; ++i) {
        NamedType *nth = implements->Nth(i);
        Decl *lookup = s->table->Lookup(implements->Nth(i)->Name());
        InterfaceDecl *intDecl = dynamic_cast<InterfaceDecl*>(lookup);

        if (intDecl == NULL)
            continue;

        List<Decl*> *intMembers = intDecl->GetMembers();

        for (int i = 0, n = intMembers->NumElements(); i < n; ++i) {
            Decl *d = intMembers->Nth(i);

            ClassDecl *classDecl = this;
            Decl *classLookup;
            while (classDecl != NULL) {
                classLookup = classDecl->GetScope()->table->Lookup(d->GetName());

                if (classLookup != NULL)
                    break;

                if (classDecl->GetExtends() == NULL) {
                    classDecl = NULL;
                } else {
                    const char *extName = classDecl->GetExtends()->GetName();
                    Decl *ext = Program::gScope->table->Lookup(extName);
                    classDecl = dynamic_cast<ClassDecl*>(ext);
                }
            }

            if (classLookup == NULL) {
                ReportError::InterfaceNotImplemented(this, nth);
                return;
            }
        }
    }
}
void ClassDecl::Check() {
    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->Check();

    CheckExtends();
    CheckImplements();

    for (int i = 0, n = implements->NumElements(); i < n; ++i)
        CheckImplementedMembers(implements->Nth(i));

    CheckExtendedMembers(extends);
    CheckImplementsInterfaces();
}

void ClassDecl::CheckAgainstScope(Scope *other) {
    Iterator<Decl*> iter = scope->table->GetIterator();
    Decl *d;
    while ((d = iter.GetNextValue()) != NULL) {
        Decl *lookup = other->table->Lookup(d->GetName());

        if (lookup == NULL)
            continue;
        //parents class define the same var with that in sub class
        if (dynamic_cast<VarDecl*>(lookup) != NULL)
            ReportError::DeclConflict(d, lookup);

        //declaration same name function but different signature in sub class
        if (dynamic_cast<FnDecl*>(lookup) != NULL &&
            !d->IsEquivalentTo(lookup))
            ReportError::OverrideMismatch(d);
    }
}

void ClassDecl::PreEmit() {
    int memOffset = CodeGenerator::OffsetToFirstField;
    int vtblOffset = CodeGenerator::OffsetToFirstMethod;

    if(extends!= nullptr) {
        Decl *d = Program::gScope->table->Lookup(extends->GetName());
        Assert(d!=NULL);
        memOffset+=d->GetMemBytes();
        vtblOffset+=d->GetVTblBytes();
    }

    for(int i=0,n=members->NumElements();i<n;++i) {
        VarDecl *d = dynamic_cast<VarDecl*>(members->Nth(i));
        if(d== nullptr)
            continue;
        d->SetMemOffset(memOffset);
        memOffset+=d->GetMemBytes();
    }

    for(int i=0,n=members->NumElements();i<n;++i) {
        FnDecl *d = dynamic_cast<FnDecl*>(members->Nth(i));
        if(d==NULL)
            continue;
        d->SetIsMethod(true);
        d->SetVTblOffset(vtblOffset);
        vtblOffset += CodeGenerator::VarSize;
    }

    for(int i=0,n=members->NumElements();i<n;++i) {
        std::string prefix;
        prefix +=GetName();
        prefix +=".";
        members->Nth(i)->AddLabelPrefix(prefix.c_str());
    }
}

Location *ClassDecl::Emit(CodeGenerator *cg) {
    for (int i=0,n=members->NumElements();i<n;++i) {
       // FnDecl* fnDecl = dynamic_cast<FnDecl*>(members->Nth(i));
       // if(fnDecl!= nullptr) continue;
        members->Nth(i)->Emit(cg);
    }
    List<FnDecl*> *decls = GetMethodDecls();
    List<const char*> *labels = new List<const char*>;
    for (int i=0,n=decls->NumElements();i<n;++i) {
        labels->Append(decls->Nth(i)->GetLabel());
    }

    cg->GenVTable(GetName(),labels);
    return nullptr;
}

List<FnDecl *> *ClassDecl::GetMethodDecls() {
    List<FnDecl*> *decls = new List<FnDecl*>;
    if(extends!=NULL) {
        Decl *d = Program::gScope->table->Lookup(extends->GetName());
        ClassDecl *c = dynamic_cast<ClassDecl*>(d);
        Assert(c!= nullptr);
        List<FnDecl*> *exDecls = c->GetMethodDecls();
        for (int i=0,n=exDecls->NumElements();i<n;++i)
            decls->Append(exDecls->Nth(i));
    }

    for(int i=0,n=members->NumElements();i<n;++i) {
        FnDecl* d = dynamic_cast<FnDecl*>(members->Nth(i));
        if(d==nullptr)
            continue;

        for(int j=0,m=decls->NumElements();j<m;++j) {
            if(strcmp(decls->Nth(j)->GetName(),d->GetName())==0) {
                decls->RemoveAt(j);
                decls->InsertAt(d,j);
            }
        }
    }

    for(int i=0,n=members->NumElements();i<n;++i) {
        FnDecl *d = dynamic_cast<FnDecl*>(members->Nth(i));
        if(d== nullptr)
            continue;
        //some same funcs are put into decls, need to revise and improve it.
        decls->Append(d);
    }
    return decls;
}

int ClassDecl::GetMemBytes() {
    int memBytes = 0;

    if (extends != NULL) {
        Decl *d = Program::gScope->table->Lookup(extends->GetName());
        Assert(d != NULL);
        memBytes += d->GetMemBytes();
    }

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        memBytes += members->Nth(i)->GetMemBytes();

    return memBytes;
}

int ClassDecl::GetVTblBytes() {
    int vtblBytes = 0;
    if(extends!= nullptr) {
        Decl *d = Program::gScope->table->Lookup(extends->GetName());
        Assert(d!= nullptr);
        vtblBytes +=d->GetVTblBytes();
    }
    for(int i=0,n=members->NumElements();i<n;++i) {
        vtblBytes+=members->Nth(i)->GetVTblBytes();
    }

    return vtblBytes;
}


InterfaceDecl::InterfaceDecl(Identifier *n, List<Decl*> *m) : Decl(n) {
    Assert(n != NULL && m != NULL);
    (members=m)->SetParentAll(this);
}

void InterfaceDecl::BuildScope() {


    for (int i = 0, n = members->NumElements(); i < n; ++i)
        scope->AddDecl(members->Nth(i));

    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->BuildScope();
}

void InterfaceDecl::Check() {
    for (int i = 0, n = members->NumElements(); i < n; ++i)
        members->Nth(i)->Check();
}


FnDecl::FnDecl(Identifier *n, Type *r, List<VarDecl*> *d) : Decl(n) {
    Assert(n != NULL && r!= NULL && d != NULL);
    (returnType=r)->SetParent(this);
    (formals=d)->SetParentAll(this);
    body = NULL;
    label = new std::string(GetName());
    if (*label != "main")
        label->insert(0, "____"); // Prefix function labels to avoid conflicts
    isMethod = false;
}

void FnDecl::SetFunctionBody(Stmt *b) { 
    (body=b)->SetParent(this);
}

bool FnDecl::IsEquivalentTo(Decl *other) {
    FnDecl *fnDecl = dynamic_cast<FnDecl*>(other);

    if (fnDecl == NULL)
        return false;

    if (!returnType->IsEquivalentTo(fnDecl->returnType))
        return false;

    if (formals->NumElements() != fnDecl->formals->NumElements())
        return false;

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        if (!formals->Nth(i)->IsEquivalentTo(fnDecl->formals->Nth(i)))
            return false;

    return true;
}

void FnDecl::BuildScope() {

    scope->SetFnDecl(this);

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        scope->AddDecl(formals->Nth(i));

    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->BuildScope();

    if (body)
        body->BuildScope();
}

void FnDecl::Check() {
    for (int i = 0, n = formals->NumElements(); i < n; ++i)
        formals->Nth(i)->Check();

    if (body)
        body->Check();
}

int FnDecl::GetVTblBytes() {
    return CodeGenerator::VarSize;
}

void FnDecl::AddLabelPrefix(const char *prefix) {
    label->insert(0,prefix);
}

Location *FnDecl::Emit(CodeGenerator *cg) {
    int offset = CodeGenerator::OffsetToFirstParam;

    if(isMethod)
        offset +=CodeGenerator::VarSize;

    for (int i=0,n=formals->NumElements();i<n;++i){
        VarDecl *d = formals->Nth(i);
        Location *loc = new Location(fpRelative,offset,d->GetName());
        d->SetMemLoc(loc);
        offset +=d->GetMemBytes();
    }

    if(body!= nullptr) {
        cg->GenLabel(GetLabel());
        cg->GenBeginFunc()->SetFrameSize(body->GetMemBytes()*4);
        body->Emit(cg);
        cg->GenEndFunc();
    }
    return nullptr;
}

const char *FnDecl::GetLabel() {
    return label->c_str();
}

bool FnDecl::HasReturnVal() {
    return returnType == Type::voidType ? 0 : 1;
}

void FnDecl::PreEmit() {

}



