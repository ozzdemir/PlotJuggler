#include "utils.h"
#include <QDebug>

std::pair<std::vector<std::string>, bool>
MoveData(PlotDataMapRef &source,
         PlotDataMapRef &destination,
         bool remove_older )
{

  std::vector<std::string> added_curves;

  auto moveDataImpl = [&](auto& source_series, auto& destination_series)
  {
    bool need_update = false;
    for (auto& it : source_series)
    {
      const std::string& ID = it.first;
      auto& source_plot = it.second;
      const std::string& plot_name = source_plot.plotName();

      auto dest_plot_it = destination_series.find(ID);
      if (dest_plot_it == destination_series.end())
      {
        added_curves.push_back(ID);

        PlotGroup::Ptr group = destination.getOrCreateGroup( source_plot.group()->name() );

        dest_plot_it =
            destination_series
            .emplace(std::piecewise_construct,
                     std::forward_as_tuple(ID),
                     std::forward_as_tuple(plot_name, group))
            .first;
        need_update = true;
      }

      auto& destination_plot = dest_plot_it->second;
      PlotGroup::Ptr destination_group = destination_plot.group();

      // copy plot attributes
      for (const auto& [name, attr]: source_plot.attributes() )
      {
        if( destination_plot.attribute(name) != attr ){
          destination_plot.setAttribute( name, attr );
          need_update = true;
        }
      }
      // Copy the group name and attributes
      if( source_plot.group() )
      {
        if( !destination_group || destination_group->name() != source_plot.group()->name() )
        {
          destination_group = destination.getOrCreateGroup( source_plot.group()->name() );
          destination_plot.changeGroup( destination_group );
        }

        for (const auto& [name, attr]: source_plot.group()->attributes() )
        {
          if( destination_group->attribute(name) != attr )
          {
            destination_group->setAttribute( name, attr );
            need_update = true;
          }
        }
      }

      if( remove_older ) {
        destination_plot.clear();
      }

      for (size_t i = 0; i < source_plot.size(); i++)
      {
        destination_plot.pushBack( source_plot.at(i) );
      }

      double max_range_x = source_plot.maximumRangeX();
      destination_plot.setMaximumRangeX(max_range_x);

      source_plot.clear();
    }
    return need_update;
  };

  //--------------------------------------------
  bool num_updated = moveDataImpl( source.numeric, destination.numeric );
  bool str_updated = moveDataImpl( source.strings, destination.strings );

  moveDataImpl( source.user_defined, destination.user_defined );

  return { added_curves, num_updated || str_updated };
}
