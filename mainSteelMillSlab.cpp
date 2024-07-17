//
// Created by Filipe De Souza on 23/12/2022.
//

#include "class/LNSModel.cpp"
//#include "class/SteelMillSlabProblem.cpp"


bool is_file_exist(std::string fileName){
    std::ifstream infile(fileName);
    return infile.good();
}

int main(int argc, char* argv[]) {

    Str instance = "2_15";
    Qtt size = 10;
    runTime = 60;
    Id selection = 9;
    Id searchInfo = 0;
    failures = 200;
    seed = 2;
    double pctRand = 0.5;

    Param param;
    for (int i=1; i<argc; ++i) {
        param = getParam(argv[i]);
        if (param.name == "testId")
            testId = param.value;
        else if (param.name == "size")
            size = (Id) std::stol(param.value);
        else if (param.name == "runTime")
            runTime = (Id) std::stol(param.value);
        else if (param.name == "selection")
            selection = (Id) std::stol(param.value);
        else if (param.name == "instance")
            instance = param.value;
        else if (param.name == "seed")
            seed = (Id) std::stol(param.value);
        else if (param.name == "failures")
            failures = (Id) std::stol(param.value);
        else if (param.name == "searchInfo")
            searchInfo = (Id) std::stol(param.value);
        else if (param.name == "pctRand")
            pctRand = (double) std::stod(param.value);
    }

    Str sseed = std::to_string(seed) + ".";
    Str stringpath = "../output/"+ testId +"/";
    int status = mkdir(stringpath.c_str(),0777);
    instancePath = "../data/bench_";
    dataPlotPath = "../output/"+ testId +"/dataPlot_";
    neighbourhoodPath = "../output/"+ testId +"/neighbourhood_";
    Str statsPath = "../output/"+ testId +"/stats_";
    improvementPath = "../output/"+ testId +"/improvement_";
    instancePath.append(instance);
    dataPlotPath.append(sseed).append(instance).append(".txt");
    statsPath.append(sseed).append(instance).append(".txt");
    neighbourhoodPath.append(sseed).append(instance).append(".csv");
    improvementPath.append(sseed).append(instance).append(".csv");
    if(is_file_exist(dataPlotPath))
        return 0;

    randNum = std::mt19937(seed);

    SteelMillSlabProblem cs = SteelMillSlabProblem(instancePath);
    cs.setpctRand(pctRand);
    cs.InitSolution();
    cs.addBestCost();
    cs.setSelection(selection,searchInfo);
    std::cout << "The objective function is: " << cs.totalCost() << std::endl;
    LNSOptions opt("SteelMillSlabProblem",&cs);
    opt.symmetry(LNSModel::SYMMETRY_BRANCHING);
    opt.symmetry(LNSModel::SYMMETRY_NONE,"none");
    opt.symmetry(LNSModel::SYMMETRY_BRANCHING,"branching");
    opt.symmetry(LNSModel::SYMMETRY_LDSB,"ldsb");
    opt.size(size);
    opt.seed(seed);
    LNSModel *m = new LNSModel(opt);

    Search::Options sopt = Search::Options();
    Search::Cutoff* c = Search::Cutoff::constant(failures);
    Search::Stop* stop = Search::Stop::time(runTime * 1000);
    sopt.stop = stop;
    sopt.cutoff = c;
    RBS<LNSModel,BAB> e(m,sopt);
    Id order,v0,v1;
    while (LNSModel *s =e.next() ){
        histImprovement hi;
        hi.changedVariable = {};
        std::cout << "The objective function is: " << s->cost().val() << "|¦¦¦¦|"<< "Iteration: " << cs.getIteration() << "|¦¦¦¦|";
        for (Id i = 0; i < cs.getQttUnAssigned(); i++) {
            order = cs.getNeighbourhood(i);
            v0 = cs.currentSlab(order);
            v1 = s->value(order);
            if (v0 != v1) {
                hi.changedVariable.push_back(order);
                cs.addChange(order);
                cs.addImprovemnt(order, cs.totalCost() - s->cost().val());
//                cs.addCostVar(order, v1);
                cs.updateSol(order, v1);
 //               std::cout << "¦|¦=" << slot << ":=" << v0 << "=>" << v1;
            }
        }
        hi.improvement = cs.totalCost() - s->cost().val();
        cs.addHistImprovement(hi);
        cs.updateCost(s->cost().val());
        std::cout << std::endl;
//s->print();
        delete s;
        if(cs.optimal()){
            break;
        }
    }

    cs.addBestCost();
    cs.writeBestCosts();
    cs.writeHistImprovements();
    cs.writeHistNeighbourhoods();
    std::cout << "Instance: " << instance << " - Heuristic: " << selection << " - Cost: " << cs.totalCost() << "|¦¦¦¦|"<< "Iteration: " << cs.getIteration() << std::endl;
    cs.pfPrintWeight();
    std::ofstream fileOut{statsPath};
    if (!fileOut.good()) {
        std::cerr << "ERROR - File not found \"" << statsPath << "\"" << std::endl;
        return 0 ;
    }
    fileOut << "Depth: " << e.statistics().depth << std::endl
            << "Fail: " << e.statistics().fail << std::endl
            << "Node: " << e.statistics().node << std::endl
            << "Nogood: " << e.statistics().nogood << std::endl
            << "Restart: " << e.statistics().restart << std::endl
            << "Propagate: " << e.statistics().propagate << std::endl
            << "Iteration: " << cs.getIteration() << std::endl;
    fileOut.close();

    cs.freeArray(selection);
    return 0;
}