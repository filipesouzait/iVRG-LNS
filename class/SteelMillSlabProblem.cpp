//
// Created by Filipe De Souza on 01/04/2022.
//

#include "SteelMillSlabProblem.h"
using namespace SteelMill;
SteelMillSlabProblem::SteelMillSlabProblem(Str fileName){
    Str str;
    int i;
    std::ifstream itemStream(fileName);
    if (!itemStream.good()) {
        std::cerr << "ERROR - File not found \"" << fileName << "\"" << std::endl;
        return;
    }

    // Read file
    getline(itemStream, str);
    std::vector<Str> line = split(str);
    inst.qttSlabSizes = (Qtt) std::stoi(line[0]);
    inst.slabSizes = (Qtt*)(calloc((inst.qttSlabSizes) , sizeof(Qtt)));
    inst.maxCapacity = (Qtt) std::stoi(line[1]);
    for(Id i=0;i<inst.qttSlabSizes;i++){
        inst.slabSizes[i] = (Qtt) std::stoi(line[i+1]);
        if(inst.slabSizes[i]>inst.maxCapacity){
            inst.maxCapacity = inst.slabSizes[i];
        }
    }
    getline(itemStream, str);
    inst.qttColor = (Qtt) std::stoi(str)+1;

    inst.orderPerColors = (Qtt*)(calloc((inst.qttColor) , sizeof(Qtt)));
    for(Id i=0;i<inst.qttColor;i++){
        inst.orderPerColors[i]=0;
    }

    getline(itemStream, str);
    inst.qttOrder = (Qtt) std::stoi(str);
    inst.qttSlabs = inst.qttOrder;
    inst.colors = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
    inst.sizes = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
    for(Id i=0;i<inst.qttOrder;i++){
        getline(itemStream, str);
        std::vector<Str> line = split(str);
        inst.colors[i] = (Qtt) std::stoi(line[1]);
        inst.sizes[i] = (Qtt) std::stoi(line[0]);
        inst.orderPerColors[inst.colors[i]]+=1;
    }

    inst.loss = (Qtt*)(calloc((inst.maxCapacity+1) , sizeof(Qtt)));
    inst.loss[0] = 0;
    int currcap = 0;
    for (int c = 1; c < inst.maxCapacity; ++c) {
        if (c > inst.slabSizes[currcap]) ++currcap;
        inst.loss[c] = inst.slabSizes[currcap] - c;
    }

    Colors = (Color*)(calloc((inst.qttColor) , sizeof(Color)));

    for(Id i=0;i<inst.qttColor;i++) {
        Color cl;
        cl.idColor = i;
        cl.orders = (Qtt*)(calloc(inst.orderPerColors[i] , sizeof(Qtt)));
        cl.qttOrders = 0;
        cl.qttOrdersAssigned = inst.orderPerColors[i];
        Colors[i]= cl;
    }
    for(Id i=0;i<inst.qttOrder;i++) {
        Id idColor = inst.colors[i];
        Color* cl = &Colors[idColor];
        cl->orders[cl->qttOrders] = i;
        cl->qttOrders +=1;
    }
};

void SteelMillSlabProblem::InitSolution(){
    inst.currentSlab = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
    inst.unassignedVars = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
    inst.neighbourhood = (Qtt*)(calloc(inst.qttOrder, sizeof(Qtt)));
    Slabs = (Slab*)(calloc((inst.qttSlabs) , sizeof(Slab)));
    usage = (Qtt*)(calloc(inst.qttOrder, sizeof(Qtt)));
    change = (Qtt*)(calloc(inst.qttOrder, sizeof(Qtt)));
    varConflict = (double*)(calloc(inst.qttOrder, sizeof(double)));
    varFailures = (double*)(calloc(inst.qttOrder, sizeof(double)));
    improvement = (Qtt*)(calloc(inst.qttOrder, sizeof(Qtt)));
    values = (Qtt*)(calloc((inst.qttSlabs) , sizeof(Qtt)));
    inst.totalCost = 0;
    for(Id i=0;i<inst.qttOrder;i++) {
        inst.currentSlab[i] = i;
        values[i] = 1;
        inst.totalCost += loss(sizes(i));
        usage[i] = 1;
        change[i] = 1;
        varConflict[i] = 1;
        varFailures[i] = 1;
        improvement[i] = 1;
        inst.unassignedVars[i] = 0;
        Slab s;
        s.idSlab=i;
        s.load=sizes(i);
        s.orderInSlab = (Qtt*)(calloc((inst.qttOrder) , sizeof(Qtt)));
        s.orderInSlab[0]=i;
        s.qttOrderInSlab=1;
        s.qttOrderAssigned=1;
        Slabs[i]=s;
    }
    updateQttAssigned();
    for (Id i=0;i<inst.qttOrder;i++) {
        relax(i);
    }
}