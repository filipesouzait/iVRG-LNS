//
// Created by Filipe De Souza on 02/05/2022.
//

#include <gecode/driver.hh>
#include <gecode/int.hh>
#include <gecode/minimodel.hh>
#include "SteelMillSlabProblem.cpp"

using namespace Gecode;

class LNSOptions : public SizeOptions {
public:
    LNSOptions(const char* p, SteelMillSlabProblem *ps) : SizeOptions(p){
        spec=ps;
    }
    //    virtual void help(void);
    SteelMillSlabProblem *problem(void) const { return spec; }
    void problem(SteelMillSlabProblem *ps) { spec=ps; }

protected:
    LNSOptions(const LNSOptions& opt) : SizeOptions(opt._name), spec(opt.spec) {}
    SteelMillSlabProblem *spec;
};
