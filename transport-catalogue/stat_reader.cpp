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
        int R = CountStops(*bus);
        int U = CountUniqueStops(*bus);
        double L = GetRouteLength(*bus);

        output << request.data() + ": "s << R << " stops on route, " << U << " unique stops, "
                << setprecision(6) << L << " route length" << endl;
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