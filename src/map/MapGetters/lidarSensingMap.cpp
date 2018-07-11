//#define DEBUGLASER
#include "lidarSensingMap.h"

lidarSensingMap::lidarSensingMap(ros::NodeHandle nh)
{
    // Init params
    ros::NodeHandle priv_nh("~");
    std::string node_name = ros::this_node::getName();
    priv_nh.param<float>(node_name+"/laser_range_min", laser_range_min, .008);
    priv_nh.param<float>(node_name+"/laser_range_max", laser_range_max, 6);
    priv_nh.param<float>(node_name+"/cell_resolution", cell_resolution, 0.25);
    priv_nh.param<int>(node_name+"/angle_offset", angle_offset, 90);

    //Init OccupancyGrid
    this->occupancy_grid = new nav_msgs::OccupancyGrid;
    this->cells_number_by_row = (2 * laser_range_max) / cell_resolution;
    this->occupancy_grid->data.resize(cells_number_by_row * cells_number_by_row);
    this->occupancy_grid->info.origin.position.x = 0;
    this->occupancy_grid->info.origin.position.y = 0;
    this->occupancy_grid->info.origin.position.z = 0;
    this->occupancy_grid->info.origin.orientation.x = 0;
    this->occupancy_grid->info.origin.orientation.y = 0;
    this->occupancy_grid->info.origin.orientation.z = 0;
    this->occupancy_grid->info.origin.orientation.w = 0;
    this->occupancy_grid->info.resolution = this->cell_resolution;
    this->occupancy_grid->info.width = this->cells_number_by_row;
    this->occupancy_grid->info.height = this->cells_number_by_row;

    //Subscriber (lidar listener)
    this->sub_laser_scan = nh.subscribe("/scan", 1, &lidarSensingMap::UpdateLidar, this);
}

void lidarSensingMap::UpdateLidar(const sensor_msgs::LaserScan &msg)
{
    std::copy(msg.ranges.begin(), msg.ranges.end(), this->laser_ranges);
}

nav_msgs::OccupancyGrid* lidarSensingMap::GetMap()
{
#ifdef DEBUG
    ROS_INFO_STREAM("Creando Metadata");
#endif
    //Updates gird
    nav_msgs::OccupancyGrid& actual_grid = *this->occupancy_grid;
    fill(actual_grid.data.begin(), actual_grid.data.end(), -1);

    //Header
    //TODO: Guardar la secuancia que se crea
    actual_grid.header.seq = 0;
    actual_grid.header.stamp = ros::Time::now();
    actual_grid.header.frame_id = "0";
    actual_grid.info.map_load_time = ros::Time::now();

    float x, y, h;
    std::tuple<int,int> gridPoint;
    auto *valid_laser_beams = new std::vector<std::tuple<int,int>>;
	for (int i = 0; i < 360; ++i)
	{
		h = laser_ranges[i];
		if (h <= laser_range_max && h > 2 * cell_resolution)
		{
			x = h * cos( (i + angle_offset) * M_PI / 180);
			y = h * sin( (i + angle_offset) * M_PI / 180);
			gridPoint = MapCoordenatesToGridSpace(x,y);
            //valid_laser_beams->push_back(gridPoint);
			WriteProbabilityOnGrid(std::get<0>(gridPoint), std::get<1>(gridPoint), 99);
		}
#ifdef DEBUGLASER
        ROS_INFO_STREAM("Datos del laser:");
        std::cout << i << "->" << this->laser_ranges[i] << std::endl;
        std::cout << "Punto en el espacio: ( " << x << ", " << y << " )" << std::endl;
        std::cout << "Grid point: ( " << std::get<0>(gridPoint) << ", " << std::get<1>(gridPoint) << " )" << std::endl;
        std::cout << "Celada del grid: " << std::get<0>(gridPoint) +  std::get<1>(gridPoint) * this->cells_number_by_row << std::endl;
#endif
	}
    //GetBeamIntersectionCells(valid_laser_beams);
    return this->occupancy_grid;
}

std::tuple<int,int> lidarSensingMap::MapCoordenatesToGridSpace(float x, float y)
{
    int x_grid = (int) ((this->cells_number_by_row / 2.0f) + (x / this->cell_resolution)); 
    int y_grid = (int) ((this->cells_number_by_row / 2.0f) + (y / this->cell_resolution)); 
    return std::make_tuple(x_grid, y_grid);
}

void lidarSensingMap::GetBeamIntersectionCells(std::vector<std::tuple<int,int>> *valid_laser_beams)
{
    //Check all intersections on y axis
    for(float x = laser_range_max; x > -laser_range_max; x--)
    {
        for(auto i = valid_laser_beams->begin(); i != valid_laser_beams->end(); i++)
        {
            float y = ((float)std::get<1>(*i) / (float)std::get<0>(*i)) * x;
            if(abs(y) < laser_range_max) 
            {
                auto gridPoint = MapCoordenatesToGridSpace(x, y);
                WriteProbabilityOnGrid(std::get<0>(gridPoint), std::get<1>(gridPoint), 99);
            }
        } 
    }
    //Check all intersections on X axis
    for(float y = laser_range_max; y > -laser_range_max; y--)
    {
        for(auto i = valid_laser_beams->begin(); i != valid_laser_beams->end(); i++)
        {
            float x = y / ((float)std::get<1>(*i) / (float)std::get<0>(*i));
            if(abs(x) < laser_range_max) 
            {
                auto gridPoint = MapCoordenatesToGridSpace(x, y);
                WriteProbabilityOnGrid(std::get<0>(gridPoint), std::get<1>(gridPoint), 99);
            }
        } 
    }
}

void lidarSensingMap::WriteProbabilityOnGrid(int grid_x, int gird_y, int p)
{
    this->occupancy_grid->data[grid_x +  gird_y * this->cells_number_by_row] = p;
#ifdef DEBUG
    ROS_INFO_STREAM("Escribiendo en celada: " << grid_x +  gird_y * this->cells_number_by_row);
#endif
}