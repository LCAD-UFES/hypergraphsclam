#ifndef HYPERGRAPHSLAM_GRAB_DATA_HPP
#define HYPERGRAPHSLAM_GRAB_DATA_HPP

#include <string>
#include <vector>
#include <mutex>
#include <set>

#include <g2o/types/slam2d/se2.h>
#include <g2o/types/slam2d/vertex_se2.h>
#include <g2o/types/slam2d/edge_se2.h>
#include <g2o/types/slam2d/types_slam2d.h>
#include <g2o/core/eigen_types.h>

#include <pcl/registration/gicp.h>

#include <StampedOdometry.hpp>
#include <StampedXSENS.hpp>
#include <StampedGPSPose.hpp>
#include <StampedGPSOrientation.hpp>
#include <StampedSICK.hpp>
#include <StampedVelodyne.hpp>
#include <StampedBumblebee.hpp>
#include <EdgeGPS.hpp>

#include <VehicleModel.hpp>
#include <LocalGridMap3D.hpp>
#include <StringHelper.hpp>
#include <Wrap2pi.hpp>

#include <matrix.h>

//#include <carmen/carmen.h>

namespace hyper {

#define MAXIMUM_VEL_SCANS 0
#define LOOP_REQUIRED_TIME 300.0
#define LOOP_REQUIRED_DISTANCE 5.0
#define ICP_THREADS_POOL_SIZE 12
#define ICP_THREAD_BLOCK_SIZE 400
#define LIDAR_ODOMETRY_MIN_DISTANCE 0.3
#define VISUAL_ODOMETRY_MIN_DISTANCE 0.1
#define ICP_TRANSLATION_CONFIDENCE_FACTOR 1.00
#define CURVATURE_REQUIRED_TIME 0.0001
#define MIN_SPEED 0.001

    // define the gicp
    using GeneralizedICP = pcl::GeneralizedIterativeClosestPoint<pcl::PointXYZHSV, pcl::PointXYZHSV>;
    // typedef pcl::GeneralizedIterativeClosestPoint<pcl::PointXYZHSV, pcl::PointXYZHSV> GeneralizedICP;

    class GrabData
    {
        private:

            // the raw input message list
            StampedMessagePtrVector raw_messages;

            // the main and general message queue
            StampedMessagePtrVector messages;

            // a gps list to help the filtering process
            StampedGPSPosePtrVector gps_messages;

            // the xsens messages
            StampedXSENSPtrVector xsens_messages;

            // a velodyne list to help the ICP process
            StampedLidarPtrVector velodyne_messages;

            // a velodyne list to help the ICP process
            StampedLidarPtrVector used_velodyne;

            // a SICK list to help the ICP process
            StampedLidarPtrVector sick_messages;

            // the used sick messages
            StampedLidarPtrVector used_sick;

            // the current cloud to be processed
            StampedLidarPtrVector *point_cloud_lidar_messages;

            // an Odometry list
            StampedOdometryPtrVector odometry_messages;

            // the bumblebee messages
            StampedBumblebeePtrVector bumblebee_messages;

            // used bumblebee messages
            StampedBumblebeePtrVector used_frames;

            // the gps origin
            Eigen::Vector2d gps_origin;

            // the current lidar message iterator index to be used in the ICP measure methods
            unsigned icp_start_index, icp_end_index;

            // mutex to avoid racing conditions
            std::mutex icp_mutex, first_last_mutex, error_increment_mutex;

            // the icp error counter
            unsigned icp_errors;

            // helper
            double dmax;

            // parameters
            unsigned maximum_vel_scans;
            double loop_required_time;
            double loop_required_distance;
            unsigned icp_threads_pool_size;
            unsigned icp_thread_block_size;
            double lidar_odometry_min_distance;
            double visual_odometry_min_distance;
            double icp_translation_confidence_factor;
            bool save_accumulated_point_clouds;
            bool save_loop_closure_point_clouds;

            bool use_velodyne_odometry;
            bool use_sick_odometry;
            bool use_bumblebee_odometry;

            bool use_velodyne_loop;
            bool use_external_velodyne_loop;
            bool use_sick_loop;
            bool use_external_sick_loop;
            bool use_bumblebee_loop;

            bool use_fake_gps;

            bool use_gps_orientation;

            bool use_restricted_loops;

            unsigned grab_data_id;

            double min_speed;

            g2o::SE2 initial_guess_pose;

            g2o::SE2 gps_error;

            // separate the gps, sick and velodyne messages
            void SeparateMessages();

            // get the gps antena position in relation to sensor board
            void SetGPSPose(std::string carmen_home);

            // get the gps estimation
            g2o::SE2 GetNearestGPSMeasure(
                        StampedMessagePtrVector::iterator it,
                        StampedMessagePtrVector::iterator end,
                        int adv,
                        double timestamp,
                        double &dt);

            // get the gps estimation
            g2o::SE2 GetGPSMeasure(
                        StampedMessagePtrVector::iterator begin,
                        StampedMessagePtrVector::iterator gps,
                        StampedMessagePtrVector::iterator end,
                        double timestamp);

            // get the gps estimation
            void GetNearestOrientation(
                        StampedMessagePtrVector::iterator it,
                        StampedMessagePtrVector::iterator end,
                        int adv,
                        double timestamp,
                        double &h,
                        double &dt);

            // get the gps full measure
            Eigen::Rotation2Dd GetGPSOrientation(
                        StampedMessagePtrVector::iterator begin,
                        StampedMessagePtrVector::iterator it,
                        StampedMessagePtrVector::iterator end,
                        double timestamp);

            // get the first gps position
            Eigen::Vector2d GetFirstGPSPosition();

            // find the nearest orientation
            double FindGPSOrientation(StampedMessagePtrVector::iterator gps);

            // iterate over the entire message list and build the measures and estimates
            void BuildGPSMeasures();

            // iterate over the entire gps list and build the angles based on gps positions
            void BuildFakeGPSMeasures();

            // iterate over the entire message list and build the measures and estimates
            void BuildOdometryMeasures();

            // build the initial estimates
            void BuildOdometryEstimates(bool gps_based, bool gps_orientation);

            // build the hacked odometry estimates
            void BuildHackedOdometryEstimates();

            // build an icp measure
            bool BuildLidarOdometryMeasure(
                    GeneralizedICP &gicp,
                    VoxelGridFilter &grid_filtering,
                    double cf,
                    const g2o::SE2 &odom,
                    PointCloudHSV::Ptr source_cloud,
                    PointCloudHSV::Ptr target_cloud,
                    g2o::SE2 &icp_measure);

            // build an icp measure
            bool BuildLidarLoopMeasure(
                    GeneralizedICP &gicp,
                    double cf,
                    double orientation,
                    PointCloudHSV::Ptr source_cloud,
                    PointCloudHSV::Ptr target_cloud,
                    g2o::SE2 &loop_measure);

            // get the next lidar block
            bool GetNextLidarBlock(unsigned &first_index, unsigned &last_index);

            // it results in a safe region
            bool GetNextICPIterators(StampedLidarPtrVector::iterator &begin, StampedLidarPtrVector::iterator &end);

            PointCloudHSV::Ptr GetFilteredPointCloud(SimpleLidarSegmentation &segm, VoxelGridFilter &grid_filtering, StampedLidarPtr lidar);

            // the main icp measure method, multithreading version
            void BuildLidarMeasuresMT();

            // build sequential and loop restriction ICP measures
            void BuildLidarOdometryMeasuresWithThreads(StampedLidarPtrVector &lidar_messages);

            // remove the unused lidar messages
            void LidarMessagesFiltering(StampedLidarPtrVector &lidar_messages, StampedLidarPtrVector &used_lidar);

            // build the gps sync lidar estimates
            void BuildLidarOdometryGPSEstimates();

            // build the lidar odometry estimates,
            // we should call this method after the BuildOdometryEstimates
            void BuildRawLidarOdometryEstimates(StampedLidarPtrVector &lidar_messages, StampedLidarPtrVector &used_lidar);

            // build the visual odometry estimates, we should call this method after the BuildOdometryEstimates
            void BuildVisualOdometryEstimates();

            // compute the loop closure measure
            void BuildLidarLoopClosureMeasures(StampedLidarPtrVector &lidar_messages);

            // compute the external loop closure measure
            bool BuildExternalLidarLoopClosureMeasures(
                StampedLidarPtrVector &internal_messages,
                StampedLidarPtrVector &external_messages,
                Eigen::Vector2d external_gps_origin,
                double external_loop_min_distance,
                double external_loop_required_distance);

            // compute the bumblebee measure
            void BuildVisualOdometryMeasures();

            // save all vertices to the external file
            void SaveAllVertices(std::ofstream &os, bool global);

            // save the odometry edges
            void SaveOdometryEdges(std::ofstream &os);

            // save the current odometry estimates to odom.txt file
            void SaveOdometryEstimates(const std::string &output_filename, bool raw_version = false);

            // save the gps edges
            void SaveGPSEdges(std::ofstream &os, bool global);

            // save the xsens edges
            void SaveXSENSEdges(std::ofstream &os);

            // save all raw gps estimates
            void SaveAllGPSEstimates(std::string filename, bool original);

            // save the gps estimates
            void SaveGPSEstimates(std::string filename, bool original);

            // save the gps lateral error, for ploting
            void SaveGPSError(std::string filename, g2o::SE2 gps_error);

            // save icp edges
            void SaveLidarEdges(const std::string &msg_name, std::ofstream &os, const StampedLidarPtrVector &lidar_messages, bool use_lidar_odometry, bool use_lidar_loop);

            // save the external lidar loop edges
            void SaveExternalLidarEdges(const std::string &msg_name, std::ofstream &os, const StampedLidarPtrVector &lidar_messages);

            // save visual odometry edges
            void SaveVisualOdometryEdges(std::ofstream &os);

            // save icp edges
            void SaveICPEdges(std::ofstream &os);

            // save the lidar estimates
            void SaveRawLidarEstimates(const std::string &filename, const StampedLidarPtrVector &lidar_messages);

            // save the visual odometry estimates
            void SaveVisualOdometryEstimates(const std::string &filename);

            // build Eigen homogeneous matrix from g2o SE2
            Eigen::Matrix4f BuildEigenMatrixFromSE2(const g2o::SE2 &transform);

            // get SE2 transform from Eigen homogeneous coordinate matrix
            g2o::SE2 GetSE2FromEigenMatrix(const Eigen::Matrix4f &matrix);

            // get SE2 transform from libviso homogeneous coordinate matrix
            g2o::SE2 GetSE2FromVisoMatrix(const Matrix &matrix);

            // removing the assignment operator overloading
            void operator=(const GrabData&);

        public:

            GrabData();

            // the main constructor
            GrabData(unsigned _gid);

            // the move constructor
            GrabData(GrabData &&gd);

            // the main destructor
            ~GrabData();

            // configuration
            void Configure(std::string config_filename, std::string carmen_ini);

            // parse the log file
            unsigned ParseLogFile(const std::string &input_filename, unsigned msg_id);

            // sync all messages and process each one
            // it builds the all estimates and measures
            // and constructs the hypergraph
            void BuildHyperGraph();

            // build external loop closures - different logs version
            void BuildExternalLoopClosures(GrabData &gd, double elmind, double elreqd);
            
            // save the hyper graph to the output file
            void SaveHyperGraph(std::ofstream &os, bool global);

            // save the external lidar loops
            void SaveExternalLidarLoopEdges(std::ofstream &os);

            // save the estimates to external files
            void SaveEstimates(std::string &base);

            // compute the odometry x gps mean absolute error
            void MeanAbsoluteErrorOdometryGPS();

            // clear the entire object
            void Clear();

    };

}

#endif
