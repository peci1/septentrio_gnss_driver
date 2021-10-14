// *****************************************************************************
//
// © Copyright 2020, Septentrio NV/SA.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//    1. Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//    2. Redistributions in binary form must reproduce the above copyright
//       notice, this list of conditions and the following disclaimer in the
//       documentation and/or other materials provided with the distribution.
//    3. Neither the name of the copyright holder nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// *****************************************************************************

// *****************************************************************************
//
// Boost Software License - Version 1.0 - August 17th, 2003
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:

// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//
// *****************************************************************************

#ifndef ROSAIC_NODE_HPP
#define ROSAIC_NODE_HPP

#ifndef ANGLE_MAX
#define ANGLE_MAX 180
#endif

#ifndef ANGLE_MIN
#define ANGLE_MIN -180
#endif

#ifndef THETA_Y_MAX
#define THETA_Y_MAX 90
#endif

#ifndef THETA_Y_MIN
#define THETA_Y_MIN -90
#endif

#ifndef LEVER_ARM_MAX
#define LEVER_ARM_MAX 100
#endif

#ifndef LEVER_ARM_MIN
#define LEVER_ARM_MIN -100
#endif

#ifndef HEADING_MAX
#define HEADING_MAX 360
#endif

#ifndef HEADING_MIN
#define HEADING_MIN -360
#endif

#ifndef PITCH_MAX
#define PITCH_MAX 90
#endif

#ifndef PITCH_MIN
#define PITCH_MIN -90
#endif

#ifndef ATTSTD_DEV_MIN
#define ATTSTD_DEV_MIN 0
#endif

#ifndef ATTSTD_DEV_MAX
#define ATTSTD_DEV_MAX 5
#endif

#ifndef POSSTD_DEV_MIN
#define POSSTD_DEV_MIN 0
#endif

#ifndef POSSTD_DEV_MAX
#define POSSTD_DEV_MAX 100
#endif
/**
 * @file rosaic_node.hpp
 * @date 21/08/20
 * @brief The heart of the ROSaic driver: The ROS node that represents it
 */

// ROS includes
#include <ros/console.h>
// Boost includes
#include <boost/regex.hpp>
// ROSaic includes
#include <septentrio_gnss_driver/communication/communication_core.hpp>

extern bool g_publish_gpgga;
extern bool g_publish_pvtcartesian;
extern bool g_publish_pvtgeodetic;
extern bool g_publish_poscovgeodetic;
extern bool g_publish_poscovcartesian;
extern bool g_publish_atteuler;
extern bool g_publish_attcoveuler;
extern bool g_publish_gpst;
extern bool g_publish_navsatfix;
//INS
extern bool g_publish_insnavcart;
extern bool g_publish_insnavgeod;
extern bool g_publish_imusetup;
extern bool g_publish_velsensorsetup;
extern bool g_publish_exteventinsnavgeod;
extern bool g_publish_exteventinsnavcart;
extern bool g_publish_extsensormeas;

extern ros::Timer g_reconnect_timer_;
extern boost::shared_ptr<ros::NodeHandle> g_nh;
extern const uint32_t g_ROS_QUEUE_SIZE;
extern std::string septentrio_receiver_type_;

/**
 * @namespace rosaic_node
 * This namespace is for the ROSaic node, handling all aspects regarding
 * ROS parameters, ROS message publishing etc.
 */
namespace rosaic_node {
    //! Handles communication with the Rx
    io_comm_rx::Comm_IO IO;

    /**
     * @brief Checks whether the parameter is in the given range
     * @param[in] val The value to check
     * @param[in] min The minimum for this value
     * @param[in] max The maximum for this value
     * @param[in] name The name of the parameter
     * @throws std::runtime_error if the parameter is out of bounds
     */
    template <typename V, typename T>
    void checkRange(V val, T min, T max, std::string name)
    {
        if (val < min || val > max)
        {
            std::stringstream ss;
            ss << "Invalid settings: " << name << " must be in range [" << min
               << ", " << max << "].";
            throw std::runtime_error(ss.str());
        }
    }

    /**
     * @brief Check whether the elements of the vector are in the given range
     * @param[in] val The vector to check
     * @param[in] min The minimum for this value
     * @param[in] max The maximum for this value
     * @param[in] name The name of the parameter
     * @throws std::runtime_error if the parameter is out of bounds
     */
    template <typename V, typename T>
    void checkRange(std::vector<V> val, T min, T max, std::string name)
    {
        for (size_t i = 0; i < val.size(); i++)
        {
            std::stringstream ss;
            ss << name << "[" << i << "]";
            checkRange(val[i], min, max, ss.str());
        }
    }

    /**
     * @brief Gets an integer or unsigned integer value from the parameter server
     * @param[in] key The key to be used in the parameter server's dictionary
     * @param[out] u Storage for the retrieved value, of type U, which can be either
     * unsigned int or int
     * @return True if found, false if not found
     */
    template <typename U>
    bool getROSInt(const std::string& key, U& u)
    {
        int param;
        if (!g_nh->getParam(key, param))
            return false;
        U min = std::numeric_limits<U>::lowest();
        U max = std::numeric_limits<U>::max();
        try
        {
            checkRange((U)param, min, max, key);
        } catch (std::runtime_error& e)
        {
            std::ostringstream ss;
            ss << e.what();
            ROS_INFO("%s", ss.str().c_str());
        }
        u = (U)param;
        return true;
    }

    /**
     * @brief Gets an integer or unsigned integer value from the parameter server
     * @param[in] key The key to be used in the parameter server's dictionary
     * @param[out] u Storage for the retrieved value, of type U, which can be either
     * unsigned int or int
     * @param[in] default_val Value to use if the server doesn't contain this
     * parameter
     * @return True if found, false if not found
     */
    template <typename U>
    void getROSInt(const std::string& key, U& u, U default_val)
    {
        if (!getROSInt(key, u))
            u = default_val;
    }

    /**
     * @class ROSaicNode
     * @brief This class represents the ROsaic node, to be extended..
     */
    class ROSaicNode
    {
    public:
        //! The constructor initializes and runs the ROSaic node, if everything works
        //! fine. It loads the user-defined ROS parameters, subscribes to Rx
        //! messages, and publishes requested ROS messages...
        ROSaicNode();

        /**
         * @brief Gets the node parameters from the ROS Parameter Server, parts of
         * which are specified in a YAML file
         *
         * The other ROSaic parameters are specified via the command line.
         */
        void getROSParams();

        /**
         * @brief Defines which Rx messages to read and which ROS messages to publish
         */
        void defineMessages();

        /**
         * @brief Configures Rx: Which SBF/NMEA messages it should output and later
         * correction settings
         */
        void configureRx();

        /**
         * @brief Initializes the I/O handling
         */
        void initializeIO();

        /**
         * @brief Sets up the stage for SBF file reading
         * @param[in] file_name The name of (or path to) the SBF file, e.g. "xyz.sbf"
         */
        void prepareSBFFileReading(std::string file_name);

        /**
         * @brief Sets up the stage for PCAP file reading
         * @param[in] file_name The path to PCAP file, e.g. "/tmp/capture.sbf"
         */
        void preparePCAPFileReading(std::string file_name);

        /**
         * @brief Attempts to (re)connect every reconnect_delay_s_ seconds
         */
        void reconnect(const ros::TimerEvent& event);

        /**
         * @brief Calls the reconnect() method
         */
        void connect();

    private:
        //! Device port
        std::string device_;
        //! Baudrate
        uint32_t baudrate_;
        //! HW flow control
        std::string hw_flow_control_;
        //! In case of serial communication to Rx, rx_serial_port_ specifies Rx's
        //! serial port connected to, e.g. USB1 or COM1
        std::string rx_serial_port_;
        //! Whether connecting to Rx was successful
        bool connected_;
        //! Datum to be used
        std::string datum_;
        //! Polling period for PVT-related SBF blocks
        uint32_t polling_period_pvt_;
        //! Polling period for all other SBF blocks and NMEA messages
        uint32_t polling_period_rest_;
        //! Delay in seconds between reconnection attempts to the connection type
        //! specified in the parameter connection_type
        float reconnect_delay_s_;
        //! Marker-to-ARP offset in the eastward direction
        float delta_e_;
        //! Marker-to-ARP offset in the northward direction
        float delta_n_;
        //! Marker-to-ARP offset in the upward direction
        float delta_u_;
        //! Antenna type, from the list returned by the command "lstAntennaInfo,
        //! Overview"
        std::string ant_type_;
        //! Serial number of your particular antenna
        std::string ant_serial_nr_;
        //! IMU orientation mode
        std::string orientation_mode_;
        //! IMU orientation mode helper variable
        bool manual_;
		//! IMU orientation x-angle
        float theta_x_;
		//! IMU orientation y-angle
        float theta_y_;
		//! IMU orientation z-angle
        float theta_z_;
        //! INS antenna lever arm x-offset
        float x_;
        //! INS antenna lever arm y-offset
        float y_;
        //! INS antenna lever arm z-offset
        float z_;
        //! INS POI offset in x-dimension
        float poi_x_;
        //! INS POI offset in y-dimension
        float poi_y_;
        //! INS POI offset in z-dimension
        float poi_z_;
        //! INS velocity sensor lever arm x-offset
        float vsm_x_;
        //! INS velocity sensor lever arm y-offset
        float vsm_y_;
        //! INS velocity sensor lever arm z-offset
        float vsm_z_;
        //! Attitude offset determination in longitudinal direction
        float heading_;
        //! Attitude offset determination in latitudinal direction
        float pitch_;
        //! INS solution reference point
        std::string ins_poi_;
        //! For heading computation when unit is powered-cycled
        std::string ins_initial_heading_;
		//! Attitude deviation mask
        float att_std_dev_;
		//! Position deviation mask
        float pos_std_dev_;
        //! Type of NTRIP connection
        std::string mode_;
        //! Hostname or IP address of the NTRIP caster to connect to
        std::string caster_;
        //! IP port number of NTRIP caster to connect to
        uint32_t caster_port_;
        //! Username for NTRIP service
        std::string username_;
        //! Password for NTRIP service
        std::string password_;
        //! Mountpoint for NTRIP service
        std::string mountpoint_;
        //! NTRIP version for NTRIP service
        std::string ntrip_version_;
        //! Whether Rx has internet or not
        bool rx_has_internet_;
        //! RTCM version for NTRIP service (if Rx does not have internet)
        std::string rtcm_version_;
        //! Rx TCP port number, e.g. 28785, on which Rx receives the corrections
        //! (can't be the same as main connection unless localhost concept is used)
        uint32_t rx_input_corrections_tcp_;
        //! Rx serial port, e.g. USB2, on which Rx receives the corrections (can't be
        //! the same as main connection unless localhost concept is used)
        std::string rx_input_corrections_serial_;
        //! Our ROS timer governing the reconnection
        ros::Timer reconnect_timer_;
        //! Whether (and at which rate) or not to send GGA to the NTRIP caster
        std::string send_gga_;
        //! Whether or not to publish the GGA message
        bool publish_gpgga_;
        //! Whether or not to publish the RMC message
        bool publish_gprmc_;
        //! Whether or not to publish the GSA message
        bool publish_gpgsa_;
        //! Whether or not to publish the GSV message
        bool publish_gpgsv_;
        //! Whether or not to publish the septentrio_gnss_driver::PVTCartesian
        //! message
        bool publish_pvtcartesian_;
        //! Whether or not to publish the septentrio_gnss_driver::PVTGeodetic message
        bool publish_pvtgeodetic_;
        //! Whether or not to publish the septentrio_gnss_driver::PosCovCartesian
        //! message
        bool publish_poscovcartesian_;
        //! Whether or not to publish the septentrio_gnss_driver::PosCovGeodetic
        //! message
        bool publish_poscovgeodetic_;
        //! Whether or not to publish the septentrio_gnss_driver::AttEuler message
        bool publish_atteuler_;
        //! Whether or not to publish the septentrio_gnss_driver::AttCovEuler message
        bool publish_attcoveuler_;
        //! Whether or not to publish the septentrio_gnss_driver::INSNavCart message
        bool publish_insnavcart_;
        //! Whether or not to publish the septentrio_gnss_driver::INSNavGeod message
        bool publish_insnavgeod_;
        //! Whether or not to publish the septentrio_gnss_driver::IMUSetup message
        bool publish_imusetup_;
        //! Whether or not to publish the septentrio_gnss_driver::VelSensorSetup message
        bool publish_velsensorsetup_;
        //! Whether or not to publish the septentrio_gnss_driver::ExtEventINSNavGeod message
        bool publish_exteventinsnavgeod_;
         //! Whether or not to publish the septentrio_gnss_driver::ExtEventINSNavCart message
        bool publish_exteventinsnavcart_;
         //! Whether or not to publish the septentrio_gnss_driver::ExtSensorMeas message
        bool publish_extsensormeas_;
        //! Since the configureRx() method should only be called once the connection
        //! was established, we need the threads to communicate this to each other.
        //! Associated mutex..
        boost::mutex connection_mutex_;
        //! Since the configureRx() method should only be called once the connection
        //! was established, we need the threads to communicate this to each other.
        //! Associated condition variable..
        boost::condition_variable connection_condition_;
        //! Host name of TCP server
        std::string tcp_host_;
        //! TCP port number
        std::string tcp_port_;
        //! Whether yet-to-be-established connection to Rx will be serial or TCP
        bool serial_;
    };
} // namespace rosaic_node

#endif // for ROSAIC_NODE_HPP
