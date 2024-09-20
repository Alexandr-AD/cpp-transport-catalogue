#include <string>
#include <iomanip>

#include "stat_reader.h"

using namespace std;

namespace printStat
{
void ParseAndPrintStat(const TransportCatalogue &tansport_catalogue, std::string_view request,
                        std::ostream &output)
{
    string s = request.data();
    s = s.substr(s.find_first_of(' ') + 1);

    if (request.substr(0, request.find_first_of(' ')) == "Bus"sv)
    {
        auto bus = tansport_catalogue.GetBus(s);
        if (bus == nullptr)
        {
            output << request.data() + ": not found"s << endl;
            return;
        }

        vector<string> bus_stat = tansport_catalogue.StatsOfBus(bus);

        output << request.data() + ": "s << bus_stat[0] << " stops on route, "s << bus_stat[1] << " unique stops, "s
                << setprecision(6) << stod(bus_stat[2]) << " route length"s << endl;
    }
    else
    {
        auto stop = tansport_catalogue.GetStop(s);
        if (stop == nullptr)
        {
            output << request.data() + ": not found"s << endl;
            return;
        }
        auto stats = tansport_catalogue.StatsOfStop(s);
        if (stats.size() == 0)
        {
            output << request.data() + ": no buses"s << endl;
            return;
        }
        output << request.data() + ": buses "s;
        for (const auto str : stats)
        {
            output << str << ' ';
        }
        output << endl;
    }
}
}