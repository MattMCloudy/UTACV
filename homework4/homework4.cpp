#include "CloudVisualizer.h"

#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/io/pcd_io.h>
#include <pcl/io/ply_io.h>
#include <pcl/common/time.h>

#include <pcl/filters/voxel_grid.h>

#include <pcl/kdtree/io.h>
#include <pcl/kdtree/kdtree_flann.h>
#include <pcl/segmentation/euclidean_cluster_comparator.h>
#include <pcl/segmentation/extract_clusters.h>

#include <pcl/sample_consensus/model_types.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/sac_model_plane.h>
#include <pcl/segmentation/sac_segmentation.h>

#define NUM_COMMAND_ARGS 1

using namespace std;

// function prototypes
void pointPickingCallback(const pcl::visualization::PointPickingEvent& event, void* cookie);
void keyboardCallback(const pcl::visualization::KeyboardEvent &event, void* viewer_void);

/***********************************************************************************************************************
* @brief callback function for handling a point picking event
* @param[in] event handle generated by the visualization window
* @param[in] cookie user data passed by the event
* @author Christoper D. McMurrough
**********************************************************************************************************************/
void pointPickingCallback(const pcl::visualization::PointPickingEvent& event, void* cookie)
{
    static int pickCount = 0;
    static pcl::PointXYZRGBA lastPoint;

    pcl::PointXYZRGBA p;
    event.getPoint(p.x, p.y, p.z);

    cout << "POINT CLICKED: " << p.x << " " << p.y << " " << p.z << endl;

    // if we have picked a point previously, compute the distance
    if(pickCount % 2 == 1)
    {
        double d = std::sqrt((p.x - lastPoint.x) * (p.x - lastPoint.x) + (p.y - lastPoint.y) * (p.y - lastPoint.y) + (p.z - lastPoint.z) * (p.z - lastPoint.z));
        cout << "DISTANCE BETWEEN THE POINTS: " << d << endl;
    }

    // update the last point and pick count
    lastPoint.x = p.x;
    lastPoint.y = p.y;
    lastPoint.z = p.z;
    pickCount++;
}

double getPointDistance(pcl::PointXYZRGBA p1, pcl::PointXYZRGBA p2) {
    return std::sqrt((p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y));
}
/***********************************************************************************************************************
* @brief callback function for handling a keypress event
* @param[in] event handle generated by the visualization window
* @param[in] viewer_void user data passed by the event
* @author Christoper D. McMurrough
**********************************************************************************************************************/
void keyboardCallback(const pcl::visualization::KeyboardEvent &event, void* viewer_void)
{
    // handle key down events
    if(event.keyDown())
    {
        // handle any keys of interest
        switch(event.getKeyCode())
        {
            case 'a':
                cout << "KEYPRESS DETECTED: '" << event.getKeySym() << "'" << endl;
                break;
            default:
                break;
        }
    }
}

/***********************************************************************************************************************
* @brief Opens a point cloud file
*
* Opens a point cloud file in either PCD or PLY format
*
* @param[out] cloudOut pointer to opened point cloud
* @param[in] filename path and name of input file
* @return false if an error occurred while opening file
* @author Christopher D. McMurrough
**********************************************************************************************************************/
bool openCloud(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr &cloudOut, const char* fileName)
{
    // convert the file name to string
    std::string fileNameStr(fileName);

    // handle various file types
    std::string fileExtension = fileNameStr.substr(fileNameStr.find_last_of(".") + 1);
    if(fileExtension.compare("pcd") == 0)
    {
        // attempt to open the file
        if(pcl::io::loadPCDFile<pcl::PointXYZRGBA>(fileNameStr, *cloudOut) == -1)
        {
            PCL_ERROR("error while attempting to read pcd file: %s \n", fileNameStr.c_str());
            return false;
        }
        else
        {
            return true;
        }
    }
    else if(fileExtension.compare("ply") == 0)
    {
        // attempt to open the file
        if(pcl::io::loadPLYFile<pcl::PointXYZRGBA>(fileNameStr, *cloudOut) == -1)
        {
            PCL_ERROR("error while attempting to read pcl file: %s \n", fileNameStr.c_str());
            return false;
        }
        else
        {
            return true;
        }
    }
    else
    {
        PCL_ERROR("error while attempting to read unsupported file: %s \n", fileNameStr.c_str());
        return false;
    }
}

/*******************************************************************************************************************//**
 * @brief Locate a plane in the cloud
 *
 * Perform planar segmentation using RANSAC, returning the plane parameters and point indices
 *
 * @param[in] cloudIn pointer to input point cloud
 * @param[out] inliers list containing the point indices of inliers
 * @param[in] distanceThreshold maximum distance of a point to the planar model to be considered an inlier
 * @param[in] maxIterations maximum number of iterations to attempt before returning
 * @return the number of inliers
 * @author Christopher D. McMurrough
 **********************************************************************************************************************/
void segmentPlane(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloudIn, pcl::PointIndices::Ptr &inliers, Eigen::Vector3f &ax, double distanceThreshold, int maxIterations)
{
    // store the model coefficients
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);

    // Create the segmentation object for the planar model and set the parameters
    pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_PLANE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setMaxIterations(maxIterations);
    seg.setDistanceThreshold(distanceThreshold);

    // Segment the largest planar component from the remaining cloud
    seg.setInputCloud(cloudIn);
    seg.segment(*inliers, *coefficients);
    ax = seg.getAxis();
}


void segmentSphere(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloudIn, pcl::PointIndices::Ptr &inliers, double distanceThreshold, int maxIterations)
{
    // store the model coefficients
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);

    // Create the segmentation object for the planar model and set the parameters
    pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_SPHERE);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setMaxIterations(maxIterations);
    seg.setDistanceThreshold(distanceThreshold);

    // Segment the largest planar component from the remaining cloud
    seg.setInputCloud(cloudIn);
    seg.segment(*inliers, *coefficients);
}

void segmentParallelPlane(const pcl::PointCloud<pcl::PointXYZRGBA>::ConstPtr &cloudIn, pcl::PointIndices::Ptr &inliers, const Eigen::Vector3f &ax, double distanceThreshold, int maxIterations)
{
    // store the model coefficients
    pcl::ModelCoefficients::Ptr coefficients(new pcl::ModelCoefficients);

    // Create the segmentation object for the planar model and set the parameters
    pcl::SACSegmentation<pcl::PointXYZRGBA> seg;
    seg.setOptimizeCoefficients(true);
    seg.setModelType(pcl::SACMODEL_PARALLEL_PLANE);
    seg.setAxis(ax);
    seg.setMethodType(pcl::SAC_RANSAC);
    seg.setMaxIterations(maxIterations);
    seg.setDistanceThreshold(distanceThreshold);

    // Segment the largest planar component from the remaining cloud
    seg.setInputCloud(cloudIn);
    seg.segment(*inliers, *coefficients);
}


pcl::PointXYZRGBA* getTopOfCluster(pcl::PointCloud<pcl::PointXYZRGBA>::Ptr &cloudFiltered, pcl::PointIndices::Ptr &cluster_inliers, double max_z) {
    double z_min = 1;
    pcl::PointXYZRGBA* top = NULL;
    for (int i = 0; i < cluster_inliers->indices.size(); i++) {
        int index = cluster_inliers->indices.at(i);

        if (cloudFiltered->points.at(index).z > max_z)
            continue;

        if (cloudFiltered->points.at(index).z < z_min) {
            z_min = cloudFiltered->points.at(index).z;
            top = &(cloudFiltered->points.at(index));
        }
    }

    return top;
}

void printDimensions(pcl::PointXYZRGBA* top_of_object, double plane_z_val, float size_cm, string obj_name) {
    double obj_size_in = (plane_z_val - top_of_object->z) * 3937.01 * size_cm;
    std::cout << setprecision(2) << obj_name << " - " << obj_size_in << "\"" << std::endl;
}

/***********************************************************************************************************************
* @brief program entry point
* @param[in] argc number of command line arguments
* @param[in] argv string array of command line arguments
* @returnS return code (0 for normal termination)
* @author Christoper D. McMurrough
**********************************************************************************************************************/
int main(int argc, char** argv)
{
    // validate and parse the command line arguments
    if(argc != NUM_COMMAND_ARGS + 1)
    {
        std::printf("USAGE: %s <file_name>\n", argv[0]);
        return 0;
    }

    // parse the command line arguments
    char* fileName = argv[1];

    // create a stop watch for measuring time
    pcl::StopWatch watch;

    // initialize the cloud viewer
    CloudVisualizer CV("Rendering Window");

    // start timing the processing step
    watch.reset();

    // open the point cloud
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudIn(new pcl::PointCloud<pcl::PointXYZRGBA>);
    openCloud(cloudIn, fileName);

    // downsample the cloud using a voxel grid filter
    const float voxelSize = 0.01;
    pcl::PointCloud<pcl::PointXYZRGBA>::Ptr cloudFiltered(new pcl::PointCloud<pcl::PointXYZRGBA>);
    pcl::VoxelGrid<pcl::PointXYZRGBA> voxFilter;
    voxFilter.setInputCloud(cloudIn);
    voxFilter.setLeafSize(static_cast<float>(voxelSize), static_cast<float>(voxelSize), static_cast<float>(voxelSize));
    voxFilter.filter(*cloudFiltered);
    std::cout << "Points before downsampling: " << cloudIn->points.size() << std::endl;
    std::cout << "Points before downsampling: " << cloudFiltered->points.size() << std::endl;

    // create the vector of indices lists (each element contains a list of imultiple indices)
    const float clusterDistance = 0.02;
    int minClusterSize = 50;
    int maxClusterSize = 100000;
    std::vector<pcl::PointIndices> clusterIndices;

    // Creating the KdTree object for the search method of the extraction
    pcl::search::KdTree<pcl::PointXYZRGBA>::Ptr tree(new pcl::search::KdTree<pcl::PointXYZRGBA>);
    tree->setInputCloud(cloudFiltered);

    // create the euclidian cluster extraction object
    pcl::EuclideanClusterExtraction<pcl::PointXYZRGBA> ec;
    ec.setClusterTolerance(clusterDistance);
    ec.setMinClusterSize(minClusterSize);
    ec.setMaxClusterSize(maxClusterSize);
    ec.setSearchMethod(tree);
    ec.setInputCloud(cloudFiltered);

    // perform the clustering
    ec.extract(clusterIndices);
    std::cout << "Clusters identified: " << clusterIndices.size() << std::endl;

    // color each cluster
    for(int i = 0; i < clusterIndices.size(); i++)
    {
        // create a random color for this cluster
        int r = 255;
        int g = 255;
        int b = 0;

        // iterate through the cluster points
        for(int j = 0; j < clusterIndices.at(i).indices.size(); j++)
        {
            cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).r = r;
            cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).g = g;
            cloudFiltered->points.at(clusterIndices.at(i).indices.at(j)).b = b;
        }
    }

    pcl::PointIndices::Ptr sphere_inliers(new pcl::PointIndices);
    segmentSphere(cloudFiltered, sphere_inliers, 0.2, 10000);

    // color the spheres blue
    for(int i = 0; i < sphere_inliers->indices.size(); i++)
    {
        int index = sphere_inliers->indices.at(i);

        // skip sphere inliers with z>0.7
        if (cloudFiltered->points.at(index).z > 0.7)
            continue;

        cloudFiltered->points.at(index).r = 0;
        cloudFiltered->points.at(index).g = 0;
        cloudFiltered->points.at(index).b = 255;
    }

    // segment a plane
    pcl::PointIndices::Ptr plane_inliers(new pcl::PointIndices);
    Eigen::Vector3f plane_axis = Eigen::Vector3f::Zero ();
    segmentPlane(cloudFiltered, plane_inliers, plane_axis, 0.0254, 5000);
    double plane_z_val = 0;
    double min_x_diff = 100;
    double min_y_diff = 100 ;
    
    /// color the plane white
    for(int i = 0; i < plane_inliers->indices.size(); i++)
    {
        int index = plane_inliers->indices.at(i);
        cloudFiltered->points.at(index).r = 255;
        cloudFiltered->points.at(index).g = 255;
        cloudFiltered->points.at(index).b = 255;

        // eliminate occurences of plane in sphere (plane wins)
        for (int j = 0; j < sphere_inliers->indices.size(); j++) {
            int sphere_index = sphere_inliers->indices.at(j);
            if (getPointDistance(cloudFiltered->points.at(sphere_index),
                cloudFiltered->points.at(index)) == 0)
            {
                sphere_inliers->indices.erase(sphere_inliers->indices.begin()+j);
            }
        }

        // find the place in the center plane where x and y are closest to center, and choose
        // that as the basis for max z value.
        if ((std::abs(cloudFiltered->points.at(index).x) < min_x_diff)
            && (std::abs(cloudFiltered->points.at(index).y) < min_y_diff)) {
            plane_z_val = cloudFiltered->points.at(index).z;
        }
    }

    std::cout << "Max Z: " << plane_z_val << std::endl;

    //segment the box tops
    pcl::PointIndices::Ptr box_inliers(new pcl::PointIndices);
    segmentParallelPlane(cloudFiltered, box_inliers, plane_axis, 0.07, 10000);

    // color the box tops green
    for(int i = 0; i < box_inliers->indices.size(); i++)
    {

        // eliminate occurences of plane in box (plane wins)
        bool point_in_other_planes = false;
        int box_index = box_inliers->indices.at(i);
        for (int j = 0; j < plane_inliers->indices.size(); j++) {
            int plane_index = plane_inliers->indices.at(j);

            if (getPointDistance(cloudFiltered->points.at(box_index), 
                cloudFiltered->points.at(plane_index)) == 0)
            {
                point_in_other_planes = true;
                box_inliers->indices.erase(box_inliers->indices.begin()+i);
            }
        }

        // eliminate occurences of box in sphere (box wins)
        for (int j = 0; j < sphere_inliers->indices.size(); j++) {
            int sphere_index = sphere_inliers->indices.at(j);

            if(getPointDistance(cloudFiltered->points.at(box_index),
                cloudFiltered->points.at(sphere_index)) == 0)
            {
                sphere_inliers->indices.erase(sphere_inliers->indices.begin()+j);
            }
        }
        
        if (!point_in_other_planes) {
            cloudFiltered->points.at(box_index).r = 0;
            cloudFiltered->points.at(box_index).g = 255;
            cloudFiltered->points.at(box_index).b = 0;
        }
    }

    for(int i = 0; i < clusterIndices.size(); i++)
    {        
        switch(cloudFiltered->points.at(clusterIndices.at(i).indices.at(50)).r) {
            case 0:
                switch(cloudFiltered->points.at(clusterIndices.at(i).indices.at(50)).g) {
                    case 0:
                        switch(cloudFiltered->points.at(clusterIndices.at(i).indices.at(50)).b) {
                            case 255:
                                // Logic for Blue Spheres
                                pcl::PointIndices::Ptr cluster_inliers(new pcl::PointIndices(clusterIndices.at(i)));
                                pcl::PointXYZRGBA* top_of_sphere = getTopOfCluster(cloudFiltered, cluster_inliers, plane_z_val);
                                if (top_of_sphere)
                                    printDimensions(top_of_sphere, plane_z_val, voxelSize, "SPHERE");
                                break;
                        }
                        break;
                    case 255:
                        switch(cloudFiltered->points.at(clusterIndices.at(i).indices.at(50)).b) {
                            case 0:
                                // Logic for Green Boxes
                                pcl::PointIndices::Ptr cluster_inliers(new pcl::PointIndices(clusterIndices.at(i)));
                                pcl::PointXYZRGBA* top_of_box = getTopOfCluster(cloudFiltered, cluster_inliers, plane_z_val);
                                if (top_of_box)
                                    printDimensions(top_of_box, plane_z_val, voxelSize, "BOX");
                                break;
                        }
                        break;
                }
                break;
        }
    }

    // get the elapsed time
    double elapsedTime = watch.getTimeSeconds();
    std::cout << elapsedTime << " seconds passed " << std::endl;

    // render the scene
    CV.addCloud(cloudFiltered);
    CV.addCoordinateFrame(cloudFiltered->sensor_origin_, cloudFiltered->sensor_orientation_);

    // register mouse and keyboard event callbacks
    CV.registerPointPickingCallback(pointPickingCallback, cloudFiltered);
    CV.registerKeyboardCallback(keyboardCallback);

    // enter visualization loop
    while(CV.isRunning())
    {
        CV.spin(100);
    }

    // exit program
    return 0;
}