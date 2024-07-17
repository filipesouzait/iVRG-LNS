//
// Created by Filipe De Souza on 01/04/2022.
//
#ifndef ROAED2012_SteelMillSlabProblem_H
#define ROAED2012_SteelMillSlabProblem_H
#include "../base/Parameter.h"

namespace SteelMill {
    struct Instance {
        Qtt qttSlabSizes;
        Qtt qttSlabs;
        Qtt totalCost;
        Qtt maxCapacity;
        Qtt qttOrder;
        Qtt qttColor;
        Qtt* colors;
        Qtt* sizes;
        Qtt* slabSizes;
        Qtt* loss;
        Qtt* currentSlab;
        Qtt* orderPerColors;
        Qtt* unassignedVars;
        Qtt* neighbourhood;
    };

    struct Color {
        Qtt idColor;
        Qtt* orders;
        Qtt qttOrders;
        Qtt qttOrdersAssigned;
    };

    struct Slab {
        Qtt idSlab;
        Qtt* orderInSlab;
        Qtt qttOrderInSlab;
        Qtt qttOrderAssigned;
        Qtt load;
    };

    struct CIG {
        Qtt* load;
        Qtt* orders;
        Qtt* orderCost;
        Qtt* orderCostAdjust;
        Qtt lowerBound;
        Qtt DiveEvery = 10;
        double alphaCIG = 0.5;
        Qtt sumCost;
    };

    struct PG {
        Qtt* closeness;
        Qtt* valCloseness;
        Qtt* unAssignedVar;
        Qtt qttUnAssignedVar;
        Qtt qttCloseness;
        Qtt IdMin;
        Qtt valMin;
        Qtt listSize = 10;
    };

    struct constraintPG {
        Qtt load;
        Qtt* color;
        Qtt qttColorUsed;
        Qtt* orders;
    };

    struct VarPG {
        Qtt qttDomainReduce;
        Qtt* propScore;
    };


    struct histImprovement {
        std::vector<Id> changedVariable;
        Cost improvement;
    };

    struct portifolio {
        double weights[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        Qtt dataCollect = 10;
        double alpha = 0.5;
        Id currentSSI = 0;
        double sumWeight = 0;
        Cost oldCost = 0;
    };

    class SteelMillSlabProblem {
    public:
        SteelMillSlabProblem(Str fileName);
        SteelMillSlabProblem();

        void InitSolution();

        Id (SteelMillSlabProblem::*getBestProcess)(Id s, Id s1);
        void (SteelMillSlabProblem::*selectSubProblem)(Qtt size);

        inline Qtt isUnassigned(Qtt o) { return inst.unassignedVars[o]!=0; }
        //inline Qtt setTotalCost(Qtt cost) { return inst.totalCost=cost; }
        inline Qtt totalCost() { return inst.totalCost; }
        inline Qtt qttSlabSizes() { return inst.qttSlabSizes; }
        inline Qtt qttSlabs() { return inst.qttSlabs; }
        inline Qtt maxCapacity() { return inst.maxCapacity; }
        inline Qtt qttOrder() { return inst.qttOrder; }
        inline Qtt qttColor() { return inst.qttColor; }
        inline Qtt colors(int i) { return inst.colors[i]; }
        inline Qtt sizes(int i) { return inst.sizes[i]; }
        inline Qtt slabSizes(int i) { return inst.slabSizes[i]; }
        inline Qtt loss(int i) { return inst.loss[i]; }
        inline Qtt* loss() { return inst.loss; }
        inline Qtt* sizes() { return inst.sizes; }
        inline Qtt currentSlab(int i) { return inst.currentSlab[i]; }
        inline Qtt* currentSlab() { return inst.currentSlab; }
        inline Qtt orderPerColors(int i) { return inst.orderPerColors[i]; }
        inline bool optimal() { return inst.totalCost == 0;}
        inline Qtt varCost(int o) { return inst.loss[Slabs[currentSlab(o)].load] - inst.loss[Slabs[currentSlab(o)].load - sizes(o)];}
        inline Qtt getIteration(){return iteration; }
        inline Qtt getQttUnAssigned(){return qttUnassigned;}
        inline Id getNeighbourhood(Id i) { return inst.neighbourhood[i]; }
        inline void updateCost(Qtt v) { inst.totalCost = v; lastImprovement=0; }
        inline void setVarConflict(Id i ,double v){ varConflict[i] = v;}
        inline void setVarFailures(Id i ,double v){ varFailures[i] = v;}
        inline void setpctRand(double v){pctRand = v;}
        inline void addUsage(Id s) { usage[s]++; }
        inline void addValues(Id o) { values[currentSlab(o)]++; }
        inline void addChange(Id s) { change[s]++; }
        inline void addImprovemnt(Id s, Id v) { improvement[s] += v; }
        inline void addBestCost() { bestCost.push_back(inst.totalCost);}
        inline void addHistImprovement(histImprovement hi) { histImprovements.push_back(hi); }
        inline void addHistNeighbourhood(){
            std::vector<Id> temp;
            for(int i=0; i < qttUnassigned; i++){
                temp.push_back(getNeighbourhood(i));
//       std::cout << getNeighbourhood(i) << " ; " ;
            }
//            std::cout << std::endl;
            histNeighbourhoods.push_back(temp);
        }
        inline void removeOrder(Id var, Id val){
            Slab* S = &Slabs[val];
            S->load -= inst.sizes[var];
            S->qttOrderInSlab -= 1;
            for(Id i=S->qttOrderInSlab;i>=0;i--){
                int order = S->orderInSlab[i];
                if(order==var){
                    S->orderInSlab[i] = S->orderInSlab[S->qttOrderInSlab];
                    break;
                }
            }
        }
        inline void addOrder(Id var, Id val){
            inst.currentSlab[var] = val;
            Slab* S = &Slabs[val];
            S->load += inst.sizes[var];
            S->orderInSlab[S->qttOrderInSlab] = var;
            S->qttOrderInSlab += 1;
        }
        inline void updateSol(Id var, Id val) {
            int currentValue = inst.currentSlab[var];
            removeOrder(var,currentValue);
            addOrder(var,val);
        }
        inline void updateQttAssigned() {
            for(Id i=0;i<inst.qttColor;i++) {
                Colors[i].qttOrdersAssigned = Colors[i].qttOrders;
            }
            for(Id i=0;i<inst.qttOrder;i++) {
                Slabs[i].qttOrderAssigned = Slabs[i].qttOrderInSlab;
                inst.unassignedVars[i]=0;
            }
            qttUnassigned = 0;
        }
        inline void setSubProblem(Qtt size) {
            pfUpdateWeight();
            updateQttAssigned();
//          writeRelatedGraph();
            ((this)->*(this->selectSubProblem))(size);
            addHistNeighbourhood();
            iteration++;
            lastImprovement++;
        }
        inline Qtt numRandom(Qtt l){return randNum()%l;}
        inline Qtt getAnAssignedRandom(){
            Qtt o = numRandom(inst.qttOrder);
            while(isUnassigned(o)){
                o = numRandom(inst.qttOrder);
            }
            return o;
        }
        inline void unassignFromSlab(Id o){
            Qtt val = currentSlab(o);
            Slab* S = &Slabs[val];
            S->qttOrderAssigned -= 1;
            for(Id i=S->qttOrderAssigned;i>=0;i--){
                int order = S->orderInSlab[i];
                if(order==o){
                    S->orderInSlab[i] = S->orderInSlab[S->qttOrderAssigned];
                    S->orderInSlab[S->qttOrderAssigned] = o;
                    break;
                }
            }
        }
        inline void unassignFromColor(Id o){
            Qtt c = colors(o);
            Color* cl = &Colors[c];
            cl->qttOrdersAssigned -= 1;
            for(Id i=cl->qttOrdersAssigned;i>=0;i--){
                int order = cl->orders[i];
                if(order==o){
                    cl->orders[i] = cl->orders[cl->qttOrdersAssigned];
                    cl->orders[cl->qttOrdersAssigned] = o;
                    break;
                }
            }
        }
        inline void relax(Qtt o){
            assert(!isUnassigned(o));
            inst.neighbourhood[qttUnassigned] = o;
            qttUnassigned++;
            values[currentSlab(o)]--;
            unassignFromSlab(o);
            unassignFromColor(o);
            inst.unassignedVars[o] = 1;
            addUsage(o);//check
        }
        inline void randomSelection(Qtt size) {
            Id o = 0;
            for (Id i = 0; i < (size); i++) {
                o = getAnAssignedRandom();
                relax(o);
            }
        }
        inline Qtt getTotalSlabRelated(Id o){
            return Slabs[currentSlab(o)].qttOrderAssigned;
        }
        inline Qtt getTotalColorRelated(Id o){
            return Colors[colors(o)].qttOrdersAssigned;
        }
        inline Qtt getTotalRelated(Id o){
            return getTotalColorRelated(o) + getTotalSlabRelated(o);
        }
        inline Qtt getVarRelated(Id o,Id i){
            if (Colors[colors(o)].qttOrdersAssigned > i){
                return Colors[colors(o)].orders[i];
            }else{
                return Slabs[currentSlab(o)].orderInSlab[i-Colors[colors(o)].qttOrdersAssigned];
            }
        }
        inline Id getVariableBasedOnChanges(Id s, Id s1){
            Id sBest = s;
            if((change[s]/(usage[s]+1.0)) < (change[s1]/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnUsage(Id s, Id s1){
            Id sBest = s;
            if(usage[s] > usage[s1]){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnCost(Id s, Id s1){
            Id sBest = s;
            if((varCost(s)/(usage[s]+1.0)) < (varCost(s1)/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnImprovement(Id s, Id s1){
            Id sBest = s;
            if((improvement[s]/(usage[s]+1.0)) < (improvement[s1]/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnLessImprovement(Id s, Id s1){
            Id sBest = s;
            if((improvement[s]*(usage[s]+1.0)) > (improvement[s1]*(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnConflict(Id s, Id s1){
            Id sBest = s;
            if((varConflict[s]/(usage[s]+1.0)) < (varConflict[s1]/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnFailures(Id s, Id s1){
            Id sBest = s;
            if((varFailures[s]/(usage[s]+1.0)) < (varFailures[s1]/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnMaxValue(Id s, Id s1){
            Id sBest = s;
            if((currentSlab(s)/(usage[s]+1.0)) < (currentSlab(s1)/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnMinValue(Id s, Id s1){
            Id sBest = s;
            if((currentSlab(s)*(usage[s]+1.0)) > (currentSlab(s1)*(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnMostComValue(Id s, Id s1){
            Id sBest = s;
            if((values[currentSlab(s)]/(usage[s]+1.0)) < (values[currentSlab(s1)]/(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnLeastComValue(Id s, Id s1){
            Id sBest = s;
            if((values[currentSlab(s)]*(usage[s]+1.0)) > (values[currentSlab(s1)]*(usage[s1]+1.0))){
                sBest = s1;
            }
            return sBest;
        }
        inline Id getVariableBasedOnRandom(Id s, Id s1){
            Id sBest = s;
            if((randNum()%2)==0){
                sBest = s1;
            }
            return sBest;
        }
        inline void VRG(Qtt size) {
            Id s;
            Qtt r;
            Id o = getAnAssignedRandom();
            relax(o);
            for (Id i = 1; i < size; i++) {
                r = getTotalRelated(o);
                if(r==0){
                    s = getAnAssignedRandom();
                }else if(r <= tournament){
                    s = getVarRelated(o,0);
                    for (Id j=1; j<r; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,j));
                    }
                }else{
                    s = getVarRelated(o, numRandom(r));
                    for (Id j=1; j<tournament; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,numRandom(r)));
                    }
                }
                o = s;
                relax(o);
            }
        }
        inline void randomPlus(Qtt size) {
            Id o = 0;
            for (Id i = 0; i < (size); i++) {
                o = getAnAssignedRandom();
                for (Id j=1; j<tournament; j++) {
                    o = ((this)->*(this->getBestProcess))(o,getAnAssignedRandom());
                }
                relax(o);
            }
        }
        inline void iVRGinterleaved(Qtt size) {
            nextSearchInfo();
            iVRG(size);
        }
        inline void iVRG(Qtt size) {
            randomPlus(size*pctRand);
            Id s;
            Qtt r,c;
            Id o = getAnAssignedRandom();
            while(qttUnassigned < size){
                r = getTotalRelated(o);
                if(r==0){
                    s = getAnAssignedRandom();
                }else if(r <= tournament){
                    c=0;
                    if(!((getTotalColorRelated(o)==0)|(getTotalSlabRelated(o)==0))){
                        if(numRandom(2)==0){
                            r=getTotalColorRelated(o);
                        }else{
                            c = getTotalColorRelated(o);
                            r = getTotalSlabRelated(o);
                        }
                    }
                    s = getVarRelated(o,c);
                    for (Id j=c+1; j<c+r; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,j));
                    }
                }else{
                    c=0;
                    if(!((getTotalColorRelated(o)==0)|(getTotalSlabRelated(o)==0))){
                        if(numRandom(2)==0){
                            r=getTotalColorRelated(o);
                        }else{
                            c = getTotalColorRelated(o);
                            r = getTotalSlabRelated(o);
                        }
                    }
                    s = getVarRelated(o, c + numRandom(r));
                    for (Id j=1; j<tournament; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,c + numRandom(r)));
                    }
                }
                o = s;
                relax(o);
            }
        }
        inline void iVRGFull(Qtt size) {
            Id s;
            Qtt r,c;
            Id o = getAnAssignedRandom();
            while(qttUnassigned < size){
                r = getTotalRelated(o);
                if((r==0)|(numRandom(tournament)==0)){
                    s = getAnAssignedRandom();
                }else if(r <= tournament){
                    c=0;
                    s = getVarRelated(o,c);
                    for (Id j=c+1; j<c+r; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,j));
                    }
                }else{
                    c=0;
                    if(!((getTotalColorRelated(o)==0)|(getTotalSlabRelated(o)==0))){
                        if(numRandom(2)==0){
                            r=getTotalColorRelated(o);
                        }else{
                            c = getTotalColorRelated(o);
                            r = getTotalSlabRelated(o);
                        }
                    }
                    s = getVarRelated(o, c + numRandom(r));
                    for (Id j=1; j<tournament; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,c + numRandom(r)));
                    }
                }
                o = s;
                relax(o);
            }
        }
        inline void iVRGNonTornament(Qtt size) {
            randomSelection(size*pctRand);
            Id s;
            Qtt r,c;
            Id o = getAnAssignedRandom();
            while(qttUnassigned < size){
                r = getTotalRelated(o);
                if(r==0){
                    s = getAnAssignedRandom();
                }else{
                    c=0;
                    if(!((getTotalColorRelated(o)==0)|(getTotalSlabRelated(o)==0))){
                        if(numRandom(2)==0){
                            r=getTotalColorRelated(o);
                        }else{
                            c = getTotalColorRelated(o);
                            r = getTotalSlabRelated(o);
                        }
                    }
                    s = getVarRelated(o,c);
                    for (Id j=c+1; j<c+r; j++) {
                        s = ((this)->*(this->getBestProcess))(s, getVarRelated(o,j));
                    }
                }
                o = s;
                relax(o);
            }
        }
        void cigUnassignAll(){
            for(Id i=0; i<inst.qttSlabs; i++){
                cig.load[i] = 0;
            }
            cig.lowerBound = 0;
            cig.sumCost = 0;
        }
        void cigAssign(Qtt o){
            Qtt s = currentSlab(o);
            cig.lowerBound -= loss(cig.load[s]);
            cig.load[s] += sizes(o);
            cig.lowerBound += loss(cig.load[s]);
        }
        void costImpactGuidedDive(){
            cigUnassignAll();
            Qtt o;
            Qtt lbBefore;
            Cost varCost,sumCost = 0;
            Qtt Qttdive = lastImprovement/cig.DiveEvery;
            std::shuffle(cig.orders, cig.orders+inst.qttOrder,std::default_random_engine(std::rand()));
            for(Qtt i=0;i<inst.qttOrder;i++){
                o=cig.orders[i];
                lbBefore = cig.lowerBound;
                cigAssign(o);
                varCost = (cig.lowerBound-lbBefore) * inst.qttOrder;
                varCost = varCost>0?varCost:0;
                cig.orderCost[o] = ((cig.orderCost[o]*Qttdive)+ varCost)/(Qttdive+1);
                sumCost+=cig.orderCost[o];
            }

            Cost Xcost = ((1-cig.alphaCIG)* (sumCost/inst.qttOrder));
            Xcost = Xcost>1 ? Xcost:1;
            for(Qtt i=0;i<inst.qttOrder;i++){
                cig.orderCostAdjust[i] = (cig.alphaCIG* cig.orderCost[i]) + Xcost;
                cig.sumCost+=cig.orderCostAdjust[i];
            }
        }
        void costImpactBase(Qtt size){
            if ((lastImprovement%cig.DiveEvery)==0){
                costImpactGuidedDive();
            }
            Cost varCost;
            Cost sumCost = cig.sumCost;
            for (Id i = 0; i < size; i++) {
                std::uniform_int_distribution<long long> distribution(0,sumCost);
                Cost v = distribution(gen);
                for (Qtt o=0; o<inst.qttOrder; o++){
                    if(!isUnassigned(o)) {
                        v = v-cig.orderCostAdjust[o];
                        if (v<=0){
                            relax(o);
                            sumCost -= cig.orderCostAdjust[o];
                            break;
                        }
                    }
                }
            }
        }
        void pgUnassignAll(){
            pg.valMin = inst.qttSlabs;
            pg.IdMin = 0;
            pg.qttCloseness = 0;
            pg.qttUnAssignedVar = inst.qttOrder;
            for(Id s=0; s<inst.qttSlabs; s++){
                cPG[s].load = 0;
                cPG[s].qttColorUsed = 0;
                for(Id c=0;c<inst.qttColor; c++){
                    cPG[s].color[c]=0;
                }
                for(Id o=0;o<inst.qttOrder; o++){
                    cPG[s].orders[o]=0;
                }
            }
            for(Id o=0; o<inst.qttOrder; o++){
                varPG[o].qttDomainReduce = 0;
            }
        }
        inline Qtt pgGetAnUnassignedRandom(){
            return pg.unAssignedVar[numRandom(pg.qttUnAssignedVar)];
        }
        inline Qtt pgGetAnUnassignedFromCloseness(){
            Id r = numRandom(pg.qttCloseness);
            pg.qttCloseness = 0;
            pg.valMin = inst.qttSlabs;
            return pg.closeness[r];
        }
        inline void pgAssignVar(Qtt o){
            Qtt s = currentSlab(o);
            pg.qttUnAssignedVar--;
            Id c = colors(o);
            if (cPG[s].color[c] == 0) {
                cPG[s].color[c]++;
                cPG[s].qttColorUsed++;
            }
            cPG[s].load = sizes(o);
            cPG[s].orders[o] = 1;
        }
        inline bool pgCheckColor(Id o,Id s){
            Id c = colors(o);
            if (cPG[s].qttColorUsed==2){
                return cPG[s].color[c]==1;
            }
            return true;
        }
        inline bool pgCheckCapacity(Id o,Id s){
            return (cPG[s].load+sizes(o)) <= maxCapacity();
        }
        inline void pgUpdateScore(Id o,Id v){
            varPG[v].qttDomainReduce++;
            varPG[o].propScore[v]++;
            varPG[v].propScore[o]++;
        }
        inline void pgUpdateMin(){
            for(Id i=0; i<pg.qttCloseness; i++){
                if(pg.valMin>pg.valCloseness[i]){
                    pg.valMin = pg.valCloseness[i];
                    pg.IdMin = i;
                }
            }
        }
        inline void pgUpdateList(Id o){
            if(pg.qttCloseness<pg.listSize){
                pg.closeness[pg.qttCloseness] = o;
                pg.valCloseness[pg.qttCloseness] = varPG[o].qttDomainReduce;
                if(pg.valMin>varPG[o].qttDomainReduce){
                    pg.valMin = varPG[o].qttDomainReduce;
                    pg.IdMin = pg.qttCloseness;
                }
                pg.qttCloseness++;
            }else if(pg.valMin<varPG[o].qttDomainReduce){
                pg.closeness[pg.IdMin] = o;
                pg.valCloseness[pg.IdMin] = varPG[o].qttDomainReduce;
                pg.valMin = varPG[o].qttDomainReduce;
                pgUpdateMin();
            }
        }
        inline void pgPropagate(Id o){
            Id v;
            Id s = currentSlab(o);
            for(Id i=pg.qttUnAssignedVar;i>=0;i--){
                v = pg.unAssignedVar[i];
                if(v==o){
                    pg.unAssignedVar[i] = pg.unAssignedVar[pg.qttUnAssignedVar];
                    pg.unAssignedVar[pg.qttUnAssignedVar] = o;
                }else if(cPG[s].orders[v]==0){
                    if(!(pgCheckColor(v,s)&pgCheckCapacity(v,s))){
                        cPG[s].orders[v]=1;
                        pgUpdateScore(o,v);
                        pgUpdateList(v);
                    }
                }
            }
        }
        void originalPGLNS(Qtt size){
            pgUnassignAll();
            Qtt o;
            while(pg.qttUnAssignedVar>size){
                if(pg.qttCloseness==0){
                    o = pgGetAnUnassignedRandom();
                }else{
                    o = pgGetAnUnassignedFromCloseness();
                }
                pgAssignVar(o);
                pgPropagate(o);
            }
            for(Id i=0; i<pg.qttUnAssignedVar; i++){
                relax(pg.unAssignedVar[i]);
            }
        }
        void pgUpdateListReversed(Id o){
            Id v;
            pg.qttUnAssignedVar--;
            for(Id i=pg.qttUnAssignedVar;i>=0;i--){
                v = pg.unAssignedVar[i];
                if(v==o){
                    pg.unAssignedVar[i] = pg.unAssignedVar[pg.qttUnAssignedVar];
                    pg.unAssignedVar[pg.qttUnAssignedVar] = o;
                }else if(varPG[o].propScore[v]>0){
                    varPG[v].qttDomainReduce = varPG[o].propScore[v];
                    pgUpdateList(v);
                }
            }
        }
        void ReversedPGLNS(Qtt size){
            pgUnassignAll();
            Qtt o;
            for(Id i=0; i<size; i++){
                if(pg.qttCloseness==0){
                    o = pgGetAnUnassignedRandom();
                }else{
                    o = pgGetAnUnassignedFromCloseness();
                }
                pgUpdateListReversed(o);
                relax(o);
            }
        }
        inline void PGLNS(Qtt size) {
            switch (iteration%3) {
                case 0:
                    originalPGLNS(size);
                    break;
                case 1:
                    ReversedPGLNS(size);
                    break;
                case 2:
                    randomSelection(size);
                    break;
            }
        }
        inline void costImpactGuidedInitialise(){
            cig.orders = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
            cig.load = (Qtt*)(calloc((inst.qttSlabs) , sizeof(Qtt)));
            cig.orderCost = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
            cig.orderCostAdjust = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
            for(int i=0; i<inst.qttOrder;i++){
                cig.orders[i] = i;
                cig.orderCostAdjust[i] = 0;
                cig.orderCost[i] = 0;
            }
        }
        inline void PGLNSInitialise(){
            varPG = (VarPG*)(calloc((inst.qttOrder) , sizeof(VarPG)));
            pg.closeness = (Qtt*)(calloc((pg.listSize) , sizeof(Qtt)));
            pg.valCloseness = (Qtt*)(calloc((pg.listSize) , sizeof(Qtt)));
            pg.unAssignedVar = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
            for(int i=0; i<inst.qttOrder;i++){
                pg.unAssignedVar[i]=i;
                varPG[i].propScore = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
                varPG[i].qttDomainReduce = 0;
                for(int o=0; o<inst.qttOrder;o++){
                    varPG[i].propScore[o] = 0;
                }
            }
            cPG = (constraintPG*)(calloc((inst.qttSlabs) , sizeof(constraintPG)));
            for(int i=0; i<inst.qttSlabs;i++){
                cPG[i].color = (Qtt*)(calloc((inst.qttColor) , sizeof(Qtt)));
                cPG[i].orders = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
            }


        }
        inline void pfUpdateWeight(){
            if(pf.oldCost==0){
                pf.oldCost = totalCost();
            }else {
                pf.sumWeight -= pf.weights[pf.currentSSI];
                Cost delta = pf.oldCost - totalCost();
                pf.weights[pf.currentSSI] = ((1 - pf.alpha) * pf.weights[pf.currentSSI]) + (pf.alpha * delta);
                pf.sumWeight += pf.weights[pf.currentSSI];
                pf.oldCost = totalCost();
            }
        }
        inline void pfPrintWeight(){
            std::cout << "weight SSI: ";
            for (Qtt s=0; s<12; s++){
                std::cout << pf.weights[s] << "; ";
            }
            std::cout << std::endl;
        }
        inline void selfAdaptativeVRG(Qtt size) {
            if (pf.dataCollect*12 > iteration) {
                nextSearchInfo();
            }else{
                if (pf.sumWeight<12){
                    pf.sumWeight = 0;
                    for (Qtt s=0; s<12; s++){
                        pf.weights[s] = pf.weights[s]*iteration;
                        pf.sumWeight += pf.weights[s];
                    }
                }
                std::uniform_int_distribution<long long> distribution(0,pf.sumWeight);
                Cost v = distribution(gen);
                for (Qtt s=0; s<12; s++){
                    v -= pf.weights[s];
                    if (v<=0){
                        pf.currentSSI = s;
                        break;
                    }
                }
            }
            iVRG(size);
        }

        inline void nextSearchInfo() {
            pf.currentSSI = iteration%11;
            switch (pf.currentSSI) {
                case 0:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnCost;
                    break;
                case 1:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnImprovement;
                    break;
                case 2:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnConflict;
                    break;
                case 3:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnUsage;
                    break;
                case 4:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnChanges;
                    break;
                case 5:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnFailures;
                    break;
                case 6:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnRandom;
                    break;
                case 7:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMaxValue;
                    break;
                case 8:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMinValue;
                    break;
                case 9:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMostComValue;
                    break;
                case 10:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnLeastComValue;
                    break;
                case 11:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnLessImprovement;
                    break;
            }
        }
        inline void setSelection(Id selection, Id searchInfo) {
            selectSubProblem = &SteelMillSlabProblem::randomSelection;
            getBestProcess = &SteelMillSlabProblem::getVariableBasedOnCost;
            switch (searchInfo) {
                case 0:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnCost;
                    break;
                case 1:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnImprovement;
                    break;
                case 2:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnConflict;
                    break;
                case 3:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnUsage;
                    break;
                case 4:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnChanges;
                    break;
                case 5:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnFailures;
                    break;
                case 6:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnRandom;
                    break;
                case 7:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMaxValue;
                    break;
                case 8:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMinValue;
                    break;
                case 9:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnMostComValue;
                    break;
                case 10:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnLeastComValue;
                    break;
                case 11:
                    getBestProcess = &SteelMillSlabProblem::getVariableBasedOnLessImprovement;
                    break;
                }

            switch (selection) {
                case 0:
                    selectSubProblem = &SteelMillSlabProblem::randomSelection;
                    break;
                case 1:
                    selectSubProblem = &SteelMillSlabProblem::VRG;
                    break;
                case 2:
                    selectSubProblem = &SteelMillSlabProblem::PGLNS;
                    PGLNSInitialise();
                    break;
                case 3:
                    selectSubProblem = &SteelMillSlabProblem::costImpactBase;
                    costImpactGuidedInitialise();
                    break;
                case 4:
                    selectSubProblem = &SteelMillSlabProblem::iVRG;
                    break;
                case 5:
                    selectSubProblem = &SteelMillSlabProblem::iVRGNonTornament;
                    break;
                case 6:
                    selectSubProblem = &SteelMillSlabProblem::randomPlus;
                    break;
                case 7:
                    selectSubProblem = &SteelMillSlabProblem::iVRGinterleaved;
                    break;
                case 8:
                    selectSubProblem = &SteelMillSlabProblem::selfAdaptativeVRG;
                    break;
                case 9:
                    selectSubProblem = &SteelMillSlabProblem::iVRGFull;
                    break;
            }
        }
        void writeHistNeighbourhoods(){
            std::ofstream fileOut{neighbourhoodPath};
            if (!fileOut.good()) {
                std::cerr << "ERROR - File not found \"" << neighbourhoodPath << "\"" << std::endl;
                return;
            }
            fileOut << "neighbourhood" << std::endl;
            for (Id i=0; i < histNeighbourhoods.size(); i++){
                fileOut << getstring(histNeighbourhoods[i]) ;
                fileOut << std::endl;
            }
            fileOut.close();
        }
        void writeHistImprovements(){
            std::ofstream fileOut{improvementPath};
            if (!fileOut.good()) {
                std::cerr << "ERROR - File not found \"" << improvementPath << "\"" << std::endl;
                return;
            }
            fileOut << "changes;"
                    << "improvement" << std::endl;
            for (Id i=0; i < histImprovements.size(); i++)
                fileOut << getstring(histImprovements[i].changedVariable) << ";"
                        << histImprovements[i].improvement << std::endl;
            fileOut.close();
        }
        void writeBestCosts(){
            std::ofstream fileOut{dataPlotPath};
            if (!fileOut.good()) {
                std::cerr << "ERROR - File not found \"" << dataPlotPath << "\"" << std::endl;
                return;
            }
            fileOut << getstring(bestCost);
            fileOut.close();
        }
        void writeRelatedGraph(){
            std::ofstream fileOut{"outputGraph.csv"};
            fileOut << "node1;"
                    << "node2" << std::endl;
            Qtt o1;
            for (Id o=0; o < inst.qttOrder; o++){
                for (Id i=0; i < getTotalRelated(o); i++) {
                    o1 = getVarRelated(o,i);
                    if (o!=o1){
                        fileOut << o << ";" << o1 << std::endl;
                    }
                }
            }
            fileOut.close();
        }
        void freeArray(Id selection){
            free(usage);
            free(change);
            free(varConflict);
            free(varFailures);
            free(improvement);
            free(values);
            free(inst.slabSizes);
            free(inst.orderPerColors);
            free(inst.colors);
            free(inst.sizes);
            free(inst.loss);
            free(inst.currentSlab);
            free(inst.unassignedVars);
            free(inst.neighbourhood);
            for(Id c=0; c<inst.qttColor; c++){
                free(Colors[c].orders);
            }
            free(Colors);
            for(Id s=0; s<inst.qttSlabs; s++){
                free(Slabs[s].orderInSlab);
            }
            free(Slabs);//12
            if(selection==3){
                free(cig.orders);
                free(cig.load);
                free(cig.orderCost);
                free(cig.orderCostAdjust);//16
            }
            if(selection==2){
                free(pg.closeness);
                free(pg.valCloseness);
                free(pg.unAssignedVar);
                for(int i=0; i<inst.qttOrder;i++){
                    free(varPG[i].propScore);
                }
                free(varPG);
                for(int i=0; i<inst.qttSlabs;i++){
                    free(cPG[i].color);
                    free(cPG[i].orders);
                }
                free(cPG);//24
            }
        }


    private:
        Instance inst;
        portifolio pf;
        PG pg;
        VarPG* varPG;
        constraintPG* cPG;
        CIG cig;
        Color* Colors;
        Slab* Slabs;
        Qtt pctRand = 0.2;
        Qtt qttUnassigned = 0;
        Qtt iteration=0;
        Qtt tournament = 5;
        Qtt lastImprovement = 0;
        Qtt* usage;
        Qtt* change;
        Qtt* improvement;
        std::vector<Id> bestCost;
        std::vector<std::vector<Id>> histNeighbourhoods = {};
        std::vector<histImprovement> histImprovements = {};
        double* varConflict;
        double* varFailures;
        Qtt* values;
    };

};
#endif //ROAED2012_CARSEQPROBLEM_H