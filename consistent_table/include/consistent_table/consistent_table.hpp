#include <map>
#include <string>
#include <fstream> // For both ofstream and ifstream.
#include <sstream>


class ConsistentTable
{
  public:
    void store(const std::string& filename);
    void load(const std::string& filename);

    std::map<int,int> m_table;
};

void ConsistentTable::store(const std::string& filename)
{
    std::ofstream file{filename};

    // Error opening.
    if (!file)
    {
        return;
    }

    // Headers.

    // Column names.
    file << "orderID,productID\n";

    for(auto it = m_table.begin(); it != m_table.end(); it++)
    {
        file << it->first << "," << it->second << "\n";
    }

    // Generated CSV file can be used to create a table in SQLite or MySQL.
}

void ConsistentTable::load(const std::string& filename)
{
    m_table.clear();

    std::ifstream file{filename};
    if (!file)
    {
        return;
    }

    // Headers.
    std::string line;
    std::getline(file, line);

    // Column names.
    while (std::getline(file, line))
    {
        std::stringstream ss{line};
        std::string orderID;
        std::getline(ss, orderID, ',');
        std::string productID;
        std::getline(ss, productID, ',');

        m_table[stoi(productID)] = stoi(orderID);
    }
}
