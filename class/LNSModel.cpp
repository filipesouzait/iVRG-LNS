//
// Created by Filipe De Souza on 30/04/2022.
//

#include "LNSOptions.cpp"

using namespace Gecode;

class SortByWeight {
public:
    /// The orders
    Qtt* orders;
    /// Initialize orders
    SortByWeight(Qtt* _orders) : orders(_orders) {}
    /// Sort order
    bool operator() (int i, int j) {
        // Order i comes before order j if the weight of i is larger than
        // the weight of j.
        return (orders[i] > orders[j]) ||
               (orders[i] == orders[j] && i<j);
    }
};

class LNSModel : public IntMinimizeScript {
protected:
    /// Problem Instance
    SteelMillSlabProblem *spec;
    /// decision variables
    IntVarArray slab, ///< Slab assigned to order i
    slabload, ///< Load of slab j
    slabcost; ///< Cost of slab j
    IntVar total_cost; ///< Total cost
    BoolVarArray boolslab;
    IntCHB chb;
    Id col = 0;
    Qtt size;
    /// Random number generator for LNS
    Rnd rnd;
    int _i=0;
public:
    enum {
        SYMMETRY_NONE,      ///< Simple symmetry
        SYMMETRY_BRANCHING, ///< Breaking symmetries with symmetry
        SYMMETRY_LDSB       ///< Use LDSB for symmetry breaking
    };
    /// Actual model
    LNSModel(const LNSOptions& opt) :
            IntMinimizeScript(opt),
            spec(opt.problem()),
            // Initialize problem variables
            slab(*this, spec->qttOrder(), 0,spec->qttSlabs()-1),
            slabload(*this, spec->qttSlabs(), 0,spec->maxCapacity()),
            slabcost(*this, spec->qttSlabs(), 0, spec->maxCapacity()),
            total_cost(*this, 0, Int::Limits::max),
            boolslab(*this,spec->qttOrder()*spec->qttSlabs()),
            chb(*this, slab),
            size(opt.size()),
            rnd(opt.seed())
    {
        // Boolean variables for slab[o]==s
     //   BoolVarArgs boolslab(spec->qttOrder()*spec->qttSlabs());
        for (unsigned int i = 0; i < spec->qttOrder(); ++i) {
            BoolVarArgs tmp(spec->qttSlabs());
            for (int j = spec->qttSlabs(); j--; ) {
                boolslab[j + i*spec->qttSlabs()] = tmp[j] = BoolVar(*this, 0, 1);
            }
            channel(*this, tmp, slab[i]);
            linear(*this, tmp, IRT_EQ,1);
        }

        // Packing constraints
        for (unsigned int s = 0; s < spec->qttSlabs(); ++s) {
            IntArgs c(spec->qttOrder());
            BoolVarArgs x(spec->qttOrder());
            for (int i=0;i < spec->qttOrder(); i++ ) {
                c[i] = spec->sizes(i);
                x[i] = boolslab[s + i*spec->qttSlabs()];
            }
            linear(*this, c, x, IRT_EQ, slabload[s]);
        }
        // Redundant packing constraint
        int totalweight = 0;
        for (unsigned int i = spec->qttOrder(); i-- ; )
            totalweight += spec->sizes(i) ;
        linear(*this, slabload, IRT_EQ, totalweight);


        // Color constraints
        IntArgs nofcolor(spec->qttColor());
        for (int c = 0; c < spec->qttColor(); c++) {
            nofcolor[c] = spec->orderPerColors(c);
        }
        BoolVar f(*this, 0, 0);
        for (unsigned int s = 0; s < spec->qttSlabs(); ++s) {
            BoolVarArgs hascolor(spec->qttColor());
            for (int c = 0; c < spec->qttColor(); c++) {
                if (nofcolor[c]) {
                    BoolVarArgs hasc(nofcolor[c]);
                    int pos = 0;
                    for (int o = 0;o < spec->qttOrder(); o++ ) {
                        if (spec->colors(o) == c)
                            hasc[pos++] = boolslab[s + o*spec->qttSlabs()];
                    }
                    assert(pos == nofcolor[c]);
                    hascolor[c] = BoolVar(*this, 0, 1);
                    rel(*this, BOT_OR, hasc, hascolor[c]);
                } else {
                    hascolor[c] = f;
                }
            }
            linear(*this, hascolor, IRT_LQ, 2);
        }

        // Compute slabcost
        IntArgs l(spec->maxCapacity()+1);
        for (int c = 0; c < spec->maxCapacity()+1; ++c) {
            l[c] = spec->loss(c);
        }

        for (int s=0; s<spec->qttSlabs(); s++) {
            element(*this, l, slabload[s], slabcost[s]);
        }
        linear(*this, slabcost, IRT_EQ, total_cost);

        // Add branching
        if (opt.symmetry() == SYMMETRY_BRANCHING) {
            // Symmetry breaking branching
            SteelMillBranch::post(*this);
        } else if (opt.symmetry() == SYMMETRY_NONE) {
            branch(*this, slab, INT_VAR_MAX_MIN(), INT_VAL_MIN());
        } else { // opt.symmetry() == SYMMETRY_LDSB
            // There is one symmetry: the values (slabs) are interchangeable.
            Symmetries syms;
            syms << ValueSymmetry(IntArgs::create(spec->qttSlabs(),0));

            // For variable order we mimic the custom brancher.  We use
            // min-size domain, breaking ties by maximum weight (preferring
            // to label larger weights earlier).  To do this, we first sort
            // (stably) by maximum weight, then use min-size domain.
            SortByWeight sbw(spec->sizes());
            IntArgs indices(spec->qttOrder());
            for (unsigned int i = 0 ; i < spec->qttOrder() ; i++)
                indices[i] = i;
            Support::quicksort(&indices[0],spec->qttOrder(),sbw);
            IntVarArgs sorted_orders(spec->qttOrder());
            for (unsigned int i = 0 ; i < spec->qttOrder() ; i++) {
                sorted_orders[i] = slab[indices[i]];
            }
            branch(*this, sorted_orders, INT_VAR_SIZE_MIN(), INT_VAL_MIN(), syms);
        }
    }
    /// Slave function for restarts
    virtual bool slave(const MetaInfo& mi) {

        if ((mi.type() == MetaInfo::RESTART) && (mi.restart() > 0)) {
            const LNSModel& l = static_cast<const LNSModel&>(*mi.last());
            for(Id i=0;i<spec->getQttUnAssigned();i++){
                Id s = spec->getNeighbourhood(i);
                spec->setVarConflict(s,chb[s]);
                spec->setVarFailures(s,slab[s].afc());
                spec->addValues(s);
            }
            spec->addBestCost();
            spec->setSubProblem(size);
            for(Id o=0;o<spec->qttOrder();o++){
                if(!spec->isUnassigned(o)) {
                    setVal(slab[o], spec->currentSlab(o));
                }
            }
            status();
            return false;
        } else {
            return true;
        }
    }

    void setVal(IntVar x, int v){
        dom(*this,x,v);
    }

    /// Constructor for cloning \a s
    LNSModel(LNSModel& s) :
            IntMinimizeScript(s), spec(s.spec), rnd(s.rnd), size(s.size), chb(s.chb) {
        slab.update(*this, s.slab);
        slabload.update(*this, s.slabload);
        slabcost.update(*this, s.slabcost);
        boolslab.update(*this,s.boolslab);
        total_cost.update(*this, s.total_cost);
    }
    /// Copy during cloning
    virtual Space*
    copy(void) {
        return new LNSModel(*this);
    }
    /// Print solution
    virtual void
    print(std::ostream& os) const {
        os << "What slab="  << slab << std::endl;
        os << "Slab load="  << slabload << std::endl;
        os << "Slab cost="  << slabcost << std::endl;
        os << "Total cost=" << total_cost << std::endl;
        int nslabsused = 0;
        int nslabscost = 0;
        bool unassigned = false;
        for (int i = spec->qttSlabs(); i--; ) {
            if (!slabload[i].assigned() || !slabcost[i].assigned()) {
                unassigned = true;
                break;
            }
            if (slabload[i].min()>0) ++nslabsused;
            if (slabcost[i].min()>0) ++nslabscost;
        }
        if (!unassigned)
            os << "Number of slabs used=" << nslabsused
               << ", slabs with cost="    << nslabscost
               << std::endl;
        os << std::endl;
    }

    /// Print solution
    virtual void
    print() const {
        std::cout << "What slab="  << slab << std::endl;
        std::cout << "Slab load="  << slabload << std::endl;
        std::cout << "Slab cost="  << slabcost << std::endl;
        std::cout << "boolslab="  << boolslab << std::endl;
        std::cout << "Total cost=" << total_cost << std::endl;
        int nslabsused = 0;
        int nslabscost = 0;
        bool unassigned = false;
        for (int i = spec->qttSlabs(); i--; ) {
            if (!slabload[i].assigned() || !slabcost[i].assigned()) {
                unassigned = true;
                break;
            }
            if (slabload[i].min()>0) ++nslabsused;
            if (slabcost[i].min()>0) ++nslabscost;
        }
        if (!unassigned)
            std::cout << "Number of slabs used=" << nslabsused
               << ", slabs with cost="    << nslabscost
               << std::endl;
        std::cout << std::endl;
    }

    /// Return solution cost
    virtual IntVar cost(void) const {
        return total_cost;
    }

    Id value(Id i){
        return slab[i].val();
    }

    class SteelMillBranch:Brancher {
    protected:
        /// Cache of first unassigned value
        mutable int start;
        /// %Choice
        class Choice : public Gecode::Choice {
        public:
            /// Position of variable
            int pos;
            /// Value of variable
            int val;
            /** Initialize choice for brancher \a b, number of
             *  alternatives \a a, position \a pos0, and value \a val0.
             */
            Choice(const Brancher& b, unsigned int a, int pos0, int val0)
                    : Gecode::Choice(b,a), pos(pos0), val(val0) {}
            /// Archive into \a e
            virtual void archive(Archive& e) const {
                Gecode::Choice::archive(e);
                e << alternatives() << pos << val;
            }
        };

        /// Construct brancher
        SteelMillBranch(Home home)
                : Brancher(home), start(0) {}
        /// Copy constructor
        SteelMillBranch(Space& home, SteelMillBranch& b)
                : Brancher(home, b), start(b.start) {
        }

    public:
        /// Check status of brancher, return true if alternatives left.
        virtual bool status(const Space& home) const {
            const LNSModel& sm = static_cast<const LNSModel&>(home);
            for (unsigned int i = start; i < sm.spec->qttOrder(); ++i)
                if (!sm.slab[i].assigned()) {
                    start = i;
                    return true;
                }
            // No non-assigned orders left
            return false;
        }
        /// Return choice
        virtual Gecode::Choice* choice(Space& home) {
            LNSModel& sm = static_cast<LNSModel&>(home);
            assert(!sm.slab[start].assigned());
            // Find order with a) minimum size, b) largest weight
            unsigned int size = sm.spec->qttOrder();
            int weight = 0;
            unsigned int pos = start;
            for (unsigned int i = start; i<sm.spec->qttOrder(); ++i) {
                if (!sm.slab[i].assigned()) {
                    if (sm.slab[i].size() == size &&
                        sm.spec->sizes(i) > weight) {
                        weight = sm.spec->sizes(i);
                        pos = i;
                    } else if (sm.slab[i].size() < size) {
                        size = sm.slab[i].size();
                        weight = sm.spec->sizes(i);
                        pos = i;
                    }
                }
            }
            unsigned int val = sm.slab[pos].min();
            // Find first still empty slab (all such slabs are symmetric)
            unsigned int firstzero = 0;
            while (firstzero < sm.spec->qttSlabs() && sm.slabload[firstzero].min() > 0)
                ++firstzero;
            assert(pos < sm.spec->qttSlabs() &&
                   val < sm.spec->qttOrder());
            return new Choice(*this, (val<firstzero) ? 2 : 1, pos, val);
        }
        virtual Choice* choice(const Space&, Archive& e) {
            unsigned int alt; int pos, val;
            e >> alt >> pos >> val;
            return new Choice(*this, alt, pos, val);
        }
        /// Perform commit for choice \a _c and alternative \a a
        virtual ExecStatus commit(Space& home, const Gecode::Choice& _c,
                                  unsigned int a) {
            LNSModel& sm = static_cast<LNSModel&>(home);
            const Choice& c = static_cast<const Choice&>(_c);
            if (a)
                return me_failed(Int::IntView(sm.slab[c.pos]).nq(home, c.val))
                       ? ES_FAILED : ES_OK;
            else
                return me_failed(Int::IntView(sm.slab[c.pos]).eq(home, c.val))
                       ? ES_FAILED : ES_OK;
        }
        /// Print explanation
        virtual void print(const Space&, const Gecode::Choice& _c,
                           unsigned int a,
                           std::ostream& o) const {
            const Choice& c = static_cast<const Choice&>(_c);
            o << "slab[" << c.pos << "] "
              << ((a == 0) ? "=" : "!=")
              << " " << c.val;
        }
        /// Copy brancher
        virtual Actor* copy(Space& home) {
            return new (home) SteelMillBranch(home, *this);
        }
        /// Post brancher
        static void post(Home home) {
            (void) new (home) SteelMillBranch(home);
        }
        /// Delete brancher and return its size
        virtual size_t dispose(Space&) {
            return sizeof(*this);
        }
    };

};
