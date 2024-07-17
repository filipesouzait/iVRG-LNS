//
// Created by Filipe souza on 02/09/2020.
//

#pragma once
#include <iterator>
#include <sys/time.h>
#include <iostream>
#include <unistd.h>
#include <functional>
#include <algorithm>
#include <sys/stat.h>
#include <cstdint>
#include <ostream>
#include <sstream>
#include <vector>
#include <iostream>
#include <fstream>
#include <climits>
#include <string>
#include <iomanip>
#include <locale>
#include <set>
#include <random>
#include<string>
#include<chrono>

namespace SteelMill
{
    typedef int64_t Cost;
    typedef int32_t Id;
    typedef int64_t Qtt;
    typedef std::string Str;

    struct Param{
        Str name;
        Str value;
    };

    Param getParam(std::string line){
        std::vector<std::string> v;
        std::stringstream ss(line);
        std::string tmp;
        while (getline(ss, tmp, '=')) {
            v.push_back(tmp);
        }
        Param p;
        if(v.size()==2){
            p.name = v[0];
            p.value = v[1];
        }else{
            p.name = "None";
        }
        return p;
    }

    std::vector<std::string> split(std::string line)
    {
        std::replace_if(line.begin(), line.end(),
                        [](char c) { return !std::isdigit(static_cast<unsigned char>(c)); },
                        ' ');
        auto newEnd = std::unique(line.begin(), line.end(),
                                  [](char a, char b) { return std::isspace(a) && std::isspace(b); });

        line.erase(newEnd, line.end());
        std::vector<std::string> v;
        std::stringstream ss(line);
        std::string tmp;
        while (getline(ss, tmp, ' ')) {
            v.push_back(tmp);
        }
        return v;
    }

    std::vector<std::string> split2(std::string line)
    {
        std::vector<std::string> v;
        std::stringstream ss(line);
        std::string tmp;
        while (getline(ss, tmp, '\t')) {
            v.push_back(tmp);
        }
        return v;
    }

    std::vector<Id> getArray(std::string line){
        std::vector<Id> intList;
        std::stringstream ss(line);
        std::string tmp;
        while (getline(ss, tmp, ',')) {
            intList.push_back((Id) std::stol(tmp));
        }
        return intList;
    }

    int32_t runTime = 120;
    int32_t seed = 1;
    Qtt failures = 200;
    Str testId = "000";
    std::string instancePath = "../data/model_";
    std::string dataPlotPath = "../solution/dataPlot_";
    std::string improvementPath = "../solution/improvement_";
    std::string neighbourhoodPath = "../solution/neighbourhood_";
    std::mt19937 randNum;
    std::uniform_real_distribution<double> dis(0.0,1.0);
    std::random_device rd;
    std::mt19937_64 gen(rd());


    std::string getstring(std::vector<Id> vec){
        std::ostringstream vts;
        if (!vec.empty()){
            std::copy(vec.begin(), vec.end()-1,
                      std::ostream_iterator<Id>(vts, ", "));
            vts << vec.back();
        }
        return vts.str();
    }

}
