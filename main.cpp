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

class Recycle : public Event
{
    void Behavior()
    {
        double co2=10.0;
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
    unsigned long numberOfCycles;
    double efficiency;
    double capacity;
    double manufaturingPrice;
    double manufacturingCO2;
    double recyclingPrice;
    double recyclingCO2;
    Battery(unsigned long numberOfCycles,double efficiency,double manufaturingPrice,double manufacturingCO2)
    {
        cycles = new Store("Počet cyklů",numberOfCycles);
        this->efficiency=efficiency;
        this->manufaturingPrice=manufaturingPrice;
        this->manufacturingCO2=manufacturingCO2;
        capacity=1.0;
    }

    void Behavior()
    {
        (new Manufacture(manufacturingCO2,manufaturingPrice))->Activate();
        while (cycles->Used()<cycles->Capacity())
        {
            Enter(*cycles,1);
            (new Charge(efficiency,capacity))->Activate();
            //capacity-=0.0005;
        }

        (new Recycle())->Activate();
    }
};

class CSVReader
{
public:
    std::vector<std::vector<std::string> > getData(std::string fileName)
    {
        std::ifstream file(fileName);

        std::vector<std::vector<std::string> > fieldOfBatteries;
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
            fieldOfBatteries.push_back(vec);

        }

        file.close();

        return fieldOfBatteries;
    }
};

void Help()
{
	std::cerr << "./ims file\n";
}

int main(int argc, char *argv[])
{
    std::string batteryName;
    unsigned long numberOfCycles;
    double efficiency;
    double manufaturingPrice;
    double manufacturingCO2;

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
    std::vector<std::vector<std::string> > fieldOfBatteries = reader.getData(fileName);
    for(std::vector<std::string> line : fieldOfBatteries)
	{
		std::string::size_type sz;
		batteryName=line[0];
		manufaturingPrice= std::stod (line[1],&sz);
		manufacturingCO2=std::stod (line[2],&sz);
		numberOfCycles=std::stoul (line[3],nullptr,0);
		efficiency=std::stod (line[4],&sz);
		efficiency/=100;

		Init(0,1000);
        (new Battery(numberOfCycles,efficiency, manufaturingPrice,manufacturingCO2))->Activate();
        Run();
        std::cout << batteryName << "\n";
        std::cout << manufaturingPrice << " Kč cena výroby.\n";
        std::cout << numberOfCycles << " počet cyklů.\n";
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
