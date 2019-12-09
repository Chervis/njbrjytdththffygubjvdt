#include "simlib.h"
#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <iterator>

#define ELECTRICITYPRICE 4.56
#define CO2PERKWH 0.513

Stat *carbonDioxide = new Stat("Amount of carbon dioxide");
Stat *price = new Stat("Price of manufacturing, liquidation and running");
Stat *electricity = new Stat("Total amount of electricity stored");

struct Data
{
    std::string batteryName;
    unsigned long numberOfCycles;
    double efficiency;
    double manufaturingPrice;
    double manufacturingCO2;
    double recyclingPrice;
    double recyclingCO2;
};

struct Capacity
{
    double x5;
    double x4;
    double x3;
    double x2;
    double x1;
};

class Recycle : public Event
{
public:
    double co2;
    double priceOfRecycling;
    Recycle(double co2,double priceOfRecycling)
    {
        this->co2=co2;
        this->priceOfRecycling=priceOfRecycling;
    }
    void Behavior()
    {
        (*price)(priceOfRecycling);
        (*carbonDioxide)(co2);
    }
};

class Charge : public Event
{
public:
    double efficiency;
    double capacity;
    Charge(double efficiency,double capacity)
    {
        this->efficiency=efficiency;
        this->capacity=capacity;
    }
    void Behavior()
    {
        double co2=(CO2PERKWH+CO2PERKWH*(1-efficiency))*capacity;
        double priceOfCharge=ELECTRICITYPRICE*capacity;
        double amountOfStoredElectricity=1*capacity;
        (*carbonDioxide)(co2);
        (*price)(priceOfCharge);
        (*electricity)(amountOfStoredElectricity);
    }
};

class Manufacture : public Event
{
public:
    double manufacturingCO2;
    double manufaturingPrice;
    Manufacture(unsigned long manufacturingCO2,double manufaturingPrice)
    {
        this->manufacturingCO2=manufacturingCO2;
        this->manufaturingPrice=manufaturingPrice;
    }
    void Behavior()
    {
        (*carbonDioxide)(manufacturingCO2);
        (*price)(manufaturingPrice);
    }
};

class Battery : public Process
{
public:
    Store *cycles;
    struct Data battery;
    struct Capacity cap;
    double capacity;
    double capacitydecrease;
    Battery(struct Data battery,struct Capacity  cap)
    {
        cycles = new Store("Počet cyklů",battery.numberOfCycles);
        this->battery=battery;
        this->cap=cap;
        capacity=1.0;
        capacitydecrease=0;
    }

    void Behavior()
    {
        (new Manufacture(battery.manufacturingCO2,battery.manufaturingPrice))->Activate();
        while (cycles->Used()<cycles->Capacity())
        {
            if (cycles->Used()==0.8*cycles->Capacity())
            {
                capacitydecrease=(capacity-cap.x5)/(cycles->Capacity()/5);
            }
            else if (cycles->Used()==0.6*cycles->Capacity())
            {
                capacitydecrease=(cap.x3-cap.x4)/(cycles->Capacity()/5);
            }
            else if (cycles->Used()==0.4*cycles->Capacity())
            {
                capacitydecrease=(cap.x2-cap.x3)/(cycles->Capacity()/5);
            }
            else if (cycles->Used()==0.2*cycles->Capacity())
            {
                capacitydecrease=(cap.x1-cap.x2)/(cycles->Capacity()/5);
            }
            else if (cycles->Used()==0)
            {
                capacitydecrease=(capacity-cap.x1)/(cycles->Capacity()/5);
            }
            Enter(*cycles,1);
            (new Charge(battery.efficiency,capacity))->Activate();
            capacity-=capacitydecrease;
        }

        (new Recycle(battery.recyclingCO2,battery.recyclingPrice))->Activate();
    }
};

class CSVReader
{
public:
    std::vector<std::vector<std::string> > getData(std::string fileName)
    {
        std::ifstream file(fileName);

        std::vector<std::vector<std::string> > fileData;
        std::string line = "";

        while (getline(file, line))
        {
            std::vector<std::string> vec;
            unsigned long i=0;

            while(i<line.length())
            {
                std::string item = "";
                while (line[i]!=',')
                {
                    item.push_back(line[i]);
                    i++;
                }
                i++;
                vec.push_back(item);
            }
            fileData.push_back(vec);

        }

        file.close();

        return fileData;
    }
};

void Help()
{
	std::cerr << "./ims file\n";
}

int main(int argc, char *argv[])
{
    struct Data  battery;
    struct Capacity  cap;

    std::string fileName;
    if (argc>1)
    {
        fileName=argv[1];
    }
    else
    {
        Help();
        return 0;
    }

    CSVReader reader;
    std::vector<std::vector<std::string> > fileData = reader.getData(fileName);
    for(std::vector<std::string> line : fileData)
	{
		std::string::size_type sz;
		battery.batteryName=line[0];
		battery.manufaturingPrice= std::stod (line[1],&sz);
		battery.manufacturingCO2=std::stod (line[2],&sz);
		battery.numberOfCycles=std::stoul (line[3],nullptr,0);
		battery.efficiency=std::stod (line[4],&sz);
		battery.recyclingPrice= std::stod (line[5],&sz);
		battery.recyclingCO2=std::stod (line[6],&sz);
		cap.x1=std::stod (line[7],&sz);
		cap.x2=std::stod (line[8],&sz);
		cap.x3=std::stod (line[9],&sz);
		cap.x4=std::stod (line[10],&sz);
		cap.x5=std::stod (line[11],&sz);

		Init(0,1);
        (new Battery(battery,cap))->Activate();
        Run();
        std::cout << battery.batteryName << "\n";
        std::cout << battery.manufaturingPrice << " Kč cena výroby.\n";
        std::cout << battery.recyclingPrice << " Kč cena likvidace.\n";
        std::cout << battery.numberOfCycles << " počet cyklů.\n";
        std::cout << (*carbonDioxide).Sum() << " kg CO2.\n";
        std::cout << (*price).Sum() << " Kč.\n";
        std::cout << (*electricity).Sum() << " kWh celkem uloženo do baterie.\n";
        std::cout << (*carbonDioxide).Sum()/(*electricity).Sum() << " kg/kWh množství CO2 na kWh uložené elektřiny.\n";
        std::cout << (*price).Sum()/(*electricity).Sum() << " Kč/kWh uložené elektřiny.\n\n";

        (*carbonDioxide).Clear();
        (*price).Clear();
        (*electricity).Clear();
	}

    return 0;
}
